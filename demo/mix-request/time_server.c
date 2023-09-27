#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "nsocket.h"
#include "isockethdr.h"

#define MAX_IBUF_LEN 1024
#define MAX_DBUF_LEN 4096


int main() {
	int sock = socket(AF_NNET, SOCK_NDP, htons(ETH_P_NIP));
	if (sock < 0) {
		printf("socket create error.\n");
		exit(1);
	}
	
	struct nsockaddr addr;
	addr.enable_mask = BIND_PREFIX;
	addr.family = AF_NNET;
	addr.prefix = "/mixsim/real/time";
	addr.plen = strlen(addr.prefix);

	int ret = bind(sock, (const struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0) {
		printf("socket bind error.\n");
		exit(1);
	}

	char recvbuf[MAX_IBUF_LEN], sendbuf[MAX_DBUF_LEN];
	struct isockbuf ibuf;
	char nbuf[1024], dbuf[2048];
	ibuf.nlen = 1024;
	ibuf.name = nbuf;
	ibuf.dlen = 2048;
	ibuf.data = dbuf;
	ibuf.type = TYPE_INTEREST;
	char content[1024];
	uint cnt = 0;
	while (1) {
		ret = irecv(sock, &ibuf, sizeof(ibuf), 0);
		if (ret < 0) {
			printf("interest recv error.\n");
			exit(1);
		}
		printf("receive ok\n");

		printf("[%u]get data: %.*s, content len=%llu, content: %.*s\n", ibuf.type, ibuf.nlen, ibuf.name, ibuf.dlen, (int)(ibuf.dlen), ibuf.data);

		int clen = sprintf(content, "current time: %d", time(NULL));

		struct isockbuf dbuf;
		dbuf.type = TYPE_DATA;
		dbuf.nlen = ibuf.nlen;
		dbuf.name = ibuf.name;
		dbuf.dlen = clen;
		dbuf.data = content;

		ret = isend(sock, &dbuf, sizeof(dbuf), 0);
		if (ret < 0) {
			printf("data send error.\n");
			exit(1);
		}
		
		cnt++;
		printf("serve count=%u\n", cnt);
	}
	close(sock);
}
