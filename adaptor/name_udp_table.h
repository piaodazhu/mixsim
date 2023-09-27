#ifndef _NAME_UDP_TABLE_
#define _NAME_UDP_TABLE_

#include <inttypes.h>
#include <arpa/inet.h>

#define MAX_NAMELEN 1024
#define MAX_ADDR_PERNAME 8
struct nut_entry {
	uint16_t name_len;
	uint16_t addr_num;
	uint16_t deleted;
	uint16_t canstop; // not enabled
	char name[MAX_NAMELEN];
	struct sockaddr_in addr[MAX_ADDR_PERNAME];
};
typedef struct nut_entry nut_entry;

struct nut {
	uint32_t table_cap;
	uint32_t entry_num;
	nut_entry *entries;
};
typedef struct nut nut;

nut*
nut_create(int init_size);

int
nut_set(nut *table, char *name, int nlen, struct sockaddr_in addr);

nut_entry*
nut_get(nut *table, char *name, int nlen);

void
nut_del(nut_entry *entry);

#endif