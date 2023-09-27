#include "adaptor.h"


int init_realtosim_udpsock(char *ip, int port)
{
	int clientfd = dyco_socket(AF_INET, SOCK_DGRAM, 0);
	if (clientfd <= 0) {
		printf("socket failed\n");
		return -1;
	}

	struct sockaddr_in serveraddr = {0};
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = inet_addr(ip);

	int result = dyco_connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (result != 0) {
		printf("connect failed\n");
		return -2;
	}

	return clientfd;
}

int init_udpsock(char *ip, int port)
{
	int clientfd = dyco_socket(AF_INET, SOCK_DGRAM, 0);
	if (clientfd <= 0) {
		printf("socket failed\n");
		return -1;
	}

	struct sockaddr_in localaddr = {0};
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(port);
	localaddr.sin_addr.s_addr = inet_addr(ip);

	int result = bind(clientfd, (struct sockaddr *)&localaddr, sizeof(localaddr));
	if (result != 0) {
		printf("bind failed\n");
		return -2;
	}

	return clientfd;
}

void udp_data_in(void *arg)
{
	int fd = *(int*)(arg);
	int length;
	char buf[4096];
	while (1) {
		length = dyco_recv(fd, buf, 4096, 0);
		if (length <= 0) {
			dyco_coroutine_sleep(0);
			continue;
		}
		printf("udp recv data length : %d\n", length);
		
		simple_name_data *data = lfq_dequeue(&freebuf_list);
		if (data == NULL) { // there is no enough free buffer. alloc it.
			data = (simple_name_data*)malloc(sizeof(simple_name_data));
		}
		
		memcpy(data, buf, length);
		// put into queue
		lfq_enqueue(&simtoreal_data_queue, data);
	}
	dyco_close(fd);
}

void udp_interest_out(void *arg)
{
	int fd = *(int*)(arg);
	int length;

	struct sockaddr_in serveraddr = {0};
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(8787);
	serveraddr.sin_addr.s_addr = inet_addr("192.168.3.168");

	while (1) {
		simple_name_data *interest = lfq_dequeue(&realtosim_interest_queue);
		if (interest == NULL) {
			dyco_coroutine_sleep(0);
			continue;
		}

		length = dyco_sendto(fd, interest, sizeof(simple_name_data), 0, (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
		lfq_enqueue(&freebuf_list, interest);
		printf("udp send interest length : %d\n", length);
	}
	dyco_close(fd);
}

void udp_interest_in(void *arg)
{
	int fd = *(int*)(arg);
	int length;
	char buf[2048];

	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(clientaddr);

	while (1) {
		length = dyco_recvfrom(fd, buf, 2048, 0, (struct sockaddr*)&clientaddr, &addrlen);
		if (length <= 0) {
			dyco_coroutine_sleep(0);
			continue;
		}
		printf("udp recv interest length : %d\n", length);
		
		simple_name_data *interest = lfq_dequeue(&freebuf_list);
		if (interest == NULL) { // there is no enough free buffer. alloc it.
			interest = (simple_name_data*)malloc(sizeof(simple_name_data));
		}

		memcpy(interest, buf, length);

		if (nut_set(nut_table, interest->buf, interest->nlen, clientaddr) == 1) { // merge repeat interest.
			// put into queue
			lfq_enqueue(&simtoreal_interest_queue, interest);
		}	
	}
	dyco_close(fd);
}

void udp_data_out(void *arg)
{
	int fd = *(int*)(arg);
	int length;
	
	while (1) {
		simple_name_data *data = lfq_dequeue(&realtosim_data_queue);
		if (data == NULL) {
			dyco_coroutine_sleep(0);
			continue;
		}

		nut_entry *entry = nut_get(nut_table, data->buf, data->nlen);
		if (entry == NULL) {
			dyco_coroutine_sleep(0);
			continue;
		}

		int i;
		for (i = 0; i < entry->addr_num; i++) {
			length = dyco_sendto(fd, data, sizeof(simple_name_data), 0, (struct sockaddr *)&entry->addr[i], sizeof(struct sockaddr_in));
			printf("udp send data length : %d, to %s:%d\n", length, inet_ntoa(entry->addr[i].sin_addr), entry->addr[i].sin_port);
		}
		nut_del(entry);
		
		lfq_enqueue(&freebuf_list, data);
	}
	dyco_close(fd);
}
