#include "adaptor.h"
#include "nsocket.h"
#include "isockethdr.h"

#define MAX_NAME_LEN 1024
#define MAX_DATA_LEN 2048
#define RWBLOCK_TIME 2000

int init_realtosim_nipsock()
{
	int clientfd = dyco_socket(AF_NNET, SOCK_NDP, htons(ETH_P_NIP));
	if (clientfd <= 0) {
		printf("socket failed\n");
		return -1;
	}

	struct nsockaddr addr;
	addr.family = AF_NNET;
        addr.enable_mask = BIND_PREFIX;
        addr.prefix = "/Rpi-1/lab1038";
	addr.plen = strlen(addr.prefix);
        int err = bind(clientfd, (const struct sockaddr*)&addr, sizeof(addr));
	if (err < 0) {
		printf("bind failed\n");
		return -2;
	}
	return clientfd;
}

int init_simtoreal_nipsock()
{
	int clientfd = dyco_socket(AF_NNET, SOCK_NDP, htons(ETH_P_NIP));
	if (clientfd <= 0) {
		printf("socket failed\n");
		return -1;
	}
	return clientfd;
}

void nip_interest_in(void *arg)
{
	int fd = *(int*)(arg);
	int length;

	char name[MAX_NAME_LEN], data[MAX_DATA_LEN];
	struct isockbuf ibuf;
	ibuf.name = name;
	ibuf.data = data;

	while (1) {
		ibuf.type = TYPE_INTEREST;
		ibuf.nlen = MAX_NAME_LEN;
		ibuf.dlen = MAX_DATA_LEN;

		do {
			int ret = dyco_coroutine_waitRead(fd, RWBLOCK_TIME);
			if (ret > 0) {
				break;
			}

			if (ret < 0) { // <= 0 : try again
				printf("wait irecv interest error! fd=%d\n", fd);
			}
		} while(1);

		length = irecv(fd, &ibuf, sizeof(ibuf), 0);
		if (length <= 0) {
			dyco_coroutine_sleep(0);
			continue;
		}

		printf("nip recv interest length : %d+%llu\n", ibuf.nlen, ibuf.dlen);
		
		simple_name_data *interest = lfq_dequeue(&freebuf_list);
		if (interest == NULL) { // there is no enough free buffer. alloc it.
			interest = (simple_name_data*)malloc(sizeof(simple_name_data));
		}

		interest->nlen = (int)ibuf.nlen;
		memcpy(interest->buf, ibuf.name, interest->nlen);
		interest->dlen = 0;

		lfq_enqueue(&realtosim_interest_queue, interest); // put into queue
	}
	dyco_close(fd);
}

void nip_data_out(void *arg)
{
	int fd = *(int*)(arg);
	int ret;

	while (1) {
		simple_name_data *databuf = lfq_dequeue(&simtoreal_data_queue);
		if (databuf == NULL) {
			dyco_coroutine_sleep(0);
			continue;
		}

		do {
			int ret = dyco_coroutine_waitWrite(fd, RWBLOCK_TIME);
			if (ret > 0) {
				break;
			}

			if (ret < 0) { // <= 0 : try again
				printf("wait isend data error! fd=%d\n", fd);
			}
		} while(1);

		struct isockbuf ibuf;
		ibuf.type = TYPE_DATA;
		ibuf.name = databuf->buf;
		ibuf.nlen = databuf->nlen;
		ibuf.data = databuf->buf + databuf->nlen;
		ibuf.dlen = databuf->dlen;
		ret = isend(fd, &ibuf, sizeof(ibuf), 0);

		lfq_enqueue(&freebuf_list, databuf);
		printf("nip send data length : %d+%llu, ret=%d\n", ibuf.nlen, ibuf.dlen, ret);
	}
	dyco_close(fd);
}


void nip_interest_out(void *arg)
{
	int fd = *(int*)(arg);
	int ret;

	while (1) {
		simple_name_data *interestbuf = lfq_dequeue(&simtoreal_interest_queue);
		if (interestbuf == NULL) {
			dyco_coroutine_sleep(0);
			continue;
		}
		printf("interestbuf name=%.*s, interestlen=%d\n", interestbuf->nlen, interestbuf->buf, interestbuf->dlen);
		printf("want send fd=%d\n", fd);

		do {
			int ret = dyco_coroutine_waitWrite(fd, RWBLOCK_TIME);
			if (ret > 0) {
				break;
			}

			if (ret < 0) { // <= 0 : try again
				printf("wait isend interest error! fd=%d\n", fd);
			}
		} while(1);

		struct isockbuf ibuf;
		ibuf.type = TYPE_INTEREST;
		ibuf.name = interestbuf->buf;
		ibuf.nlen = interestbuf->nlen;
		ibuf.data = interestbuf->buf + interestbuf->nlen;
		ibuf.dlen = 0;
		ret = isend(fd, &ibuf, sizeof(ibuf), 0);

		lfq_enqueue(&freebuf_list, interestbuf);
		printf("nip send interest length :%d+%llu, ret=%d\n", ibuf.nlen, ibuf.dlen, ret);
	}
	dyco_close(fd);
}

void nip_data_in(void *arg)
{
	int fd = *(int*)(arg);
	int length;

	char name[MAX_NAME_LEN], data[MAX_DATA_LEN];
	struct isockbuf ibuf;
	ibuf.name = name;
	ibuf.data = data;

	while (1) {
		ibuf.type = TYPE_DATA;
		ibuf.nlen = MAX_NAME_LEN;
		ibuf.dlen = MAX_DATA_LEN;

		do {
			int ret = dyco_coroutine_waitRead(fd, RWBLOCK_TIME);
			if (ret > 0) {
				break;
			}

			if (ret < 0) { // <= 0 : try again
				printf("wait irecv data error! fd=%d\n", fd);
			}
		} while(1);

		length = irecv(fd, &ibuf, sizeof(ibuf), 0);
		if (length <= 0) {
			dyco_coroutine_sleep(0);
			continue;
		}
		printf("nip recv data length : %d+%llu\n", ibuf.nlen, ibuf.dlen);
		
		simple_name_data *data = lfq_dequeue(&freebuf_list);
		if (data == NULL) { // there is no enough free buffer. alloc it.
			data = (simple_name_data*)malloc(sizeof(simple_name_data));
		}
		data->nlen = (int)ibuf.nlen;

		memcpy(data->buf, ibuf.name, data->nlen);
		if (ibuf.d_type == D_NACK) { // nack --> empty content
			data->dlen = 0;
		} else {
			data->dlen = (int)ibuf.dlen;
			memcpy(data->buf + data->nlen, ibuf.data, data->dlen);
		}
		
		// printf("[?] fd=%d recv nipdata[type=%d], name=%.*s, dlen=%llu\n", fd, ibuf.type, (int)ibuf.nlen, ibuf.name, ibuf.dlen);
		lfq_enqueue(&realtosim_data_queue, data); // put into queue
	}
	dyco_close(fd);
}
