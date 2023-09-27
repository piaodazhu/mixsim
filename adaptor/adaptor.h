#ifndef ADAPTOR_H_
#define ADAPTOR_H_
#include <stdio.h>
#include <arpa/inet.h>
#include "dyco/dyco_coroutine.h"
#include "lfq.h"
#include "name_udp_table.h"

extern struct lfq_ctx realtosim_interest_queue, simtoreal_interest_queue, simtoreal_data_queue, realtosim_data_queue;
extern struct lfq_ctx freebuf_list;

extern struct nut *nut_table;

struct simple_name_data {
	unsigned int nlen;
	unsigned int dlen;
	char buf[2040];
};

typedef struct simple_name_data simple_name_data;

void request_test();

int init_simtoreal_nipsock();
int init_realtosim_nipsock();
void nip_interest_in(void *arg);
void nip_data_out(void *arg);
void nip_data_in(void *arg);
void nip_interest_out(void *arg);

int init_realtosim_udpsock(char *ip, int port);
int init_udpsock(char *ip, int port);
void udp_data_in(void *arg);
void udp_interest_out(void *arg);
void udp_interest_in(void *arg);
void udp_data_out(void *arg);

#endif