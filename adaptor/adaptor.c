#include "adaptor.h"

#define STACK_SIZE (1024*1024)

struct nut *nut_table;
struct lfq_ctx realtosim_interest_queue, simtoreal_interest_queue, simtoreal_data_queue, realtosim_data_queue;
struct lfq_ctx freebuf_list;

void init_adaptor()
{
	nut_table = nut_create(64);

	lfq_init(&freebuf_list, 1);
	void *freeblock = malloc(sizeof(simple_name_data) * 16);
	int i;
	for (i = 0; i < 16; i++) {
		lfq_enqueue(&freebuf_list, freeblock + sizeof(simple_name_data) * i);
	}

	lfq_init(&realtosim_interest_queue, 1);
	int *realtosim_udpsock = (int*)malloc(sizeof(int));
	*realtosim_udpsock = init_udpsock("0.0.0.0", 8797);

	lfq_init(&simtoreal_interest_queue, 1);
	int *simtoreal_udpsock = (int*)malloc(sizeof(int));
	*simtoreal_udpsock = init_udpsock("0.0.0.0", 8798);
	
	lfq_init(&simtoreal_data_queue, 1);
	int *simtoreal_nipsock = (int*)malloc(sizeof(int));
	*simtoreal_nipsock = init_simtoreal_nipsock();

	lfq_init(&realtosim_data_queue, 1);
	int *realtosim_nipsock = (int*)malloc(sizeof(int));
	*realtosim_nipsock = init_realtosim_nipsock();

	dyco_coroutine_setStack(dyco_coroutine_create(udp_interest_out, realtosim_udpsock), malloc(STACK_SIZE), STACK_SIZE);
	dyco_coroutine_setStack(dyco_coroutine_create(udp_interest_in, simtoreal_udpsock), malloc(STACK_SIZE), STACK_SIZE);
	dyco_coroutine_setStack(dyco_coroutine_create(udp_data_out, simtoreal_udpsock), malloc(STACK_SIZE), STACK_SIZE);
	dyco_coroutine_setStack(dyco_coroutine_create(udp_data_in, realtosim_udpsock), malloc(STACK_SIZE), STACK_SIZE);

	dyco_coroutine_setStack(dyco_coroutine_create(nip_interest_out, simtoreal_nipsock), malloc(STACK_SIZE), STACK_SIZE);
	dyco_coroutine_setStack(dyco_coroutine_create(nip_interest_in, realtosim_nipsock), malloc(STACK_SIZE), STACK_SIZE);
	dyco_coroutine_setStack(dyco_coroutine_create(nip_data_out, realtosim_nipsock), malloc(STACK_SIZE), STACK_SIZE);
	dyco_coroutine_setStack(dyco_coroutine_create(nip_data_in, simtoreal_nipsock), malloc(STACK_SIZE), STACK_SIZE);
}

int main() {
	init_adaptor();
	dyco_schedule_run();
	return 0;
}
