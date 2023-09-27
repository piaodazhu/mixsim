#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "nsocket.h"
#include "isockethdr.h"

#define MAX_IBUF_LEN 1024
#define MAX_DBUF_LEN 4096

int main() {
	int sock = socket(AF_NNET, SOCK_NDP, htons(ETH_P_NIP));
	if (sock < 0) {
		printf("cannot open nip socket.");
		exit(1);
	}

	char *reqname = "/Rpi-1/lab1038/info/about/10Mfile.txt/3333";

	char name[MAX_IBUF_LEN], data[MAX_DBUF_LEN];
	struct isockbuf ibuf;
	ibuf.type = TYPE_INTEREST;
	ibuf.nlen = strlen(reqname);
	ibuf.name = reqname;

	int ierr = isend(sock, &ibuf, sizeof(ibuf), 0);
	if (ierr < 0) {
		printf("cannot send interest.");
		exit(1);
	}
	printf("send interest: %s\n", reqname);
	
	struct isockbuf dbuf;
	dbuf.type = TYPE_DATA;
	dbuf.nlen = MAX_IBUF_LEN;
	dbuf.name = name;
	dbuf.dlen = MAX_DBUF_LEN;
	dbuf.data = data;

	int derr = irecv(sock, &dbuf, sizeof(dbuf), 0);
	if (derr < 0) {
		printf("cannot receive data.");
		exit(1);
	}

	struct isockbuf pkt = dbuf;
	printf("[%u]get data: %.*s, content len=%llu, content: %.*s\n", pkt.type, pkt.nlen, pkt.name, pkt.dlen, (int)(pkt.dlen), pkt.data);
}
