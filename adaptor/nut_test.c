#include "name_udp_table.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main() {
	struct sockaddr_in addr[10];
	int i;
	for (i = 0; i < 10; i++) {
		addr[i].sin_port = i;
	}

	nut* table = nut_create(4);
	assert(nut_set(table, "prefix1", strlen("prefix1"), addr[1]) == 1);
	assert(nut_set(table, "prefix1", strlen("prefix1"), addr[2]) == 1);
	assert(nut_set(table, "prefix2", strlen("prefix2"), addr[3]) == 1);
	assert(nut_set(table, "prefix2", strlen("prefix2"), addr[4]) == 1);
	assert(nut_set(table, "prefix1", strlen("prefix1"), addr[1]) == 0);

	nut_entry *entry = nut_get(table, "prefix1", strlen("prefix1"));
	assert(entry->addr_num == 2);
	assert(entry->addr[0].sin_port == 1 && entry->addr[1].sin_port == 2);

	nut_del(entry);

	entry = nut_get(table, "prefix2", strlen("prefix2"));
	assert(entry->addr[0].sin_port == 3 && entry->addr[1].sin_port == 4);

	entry = nut_get(table, "prefix1", strlen("prefix1"));
	assert(entry == NULL);

	assert(nut_set(table, "prefix3", strlen("prefix3"), addr[5]) == 1);
	assert(table->entry_num == 2 && table->table_cap == 4);
	assert(nut_set(table, "prefix4", strlen("prefix4"), addr[6]) == 1);
	assert(nut_set(table, "prefix5", strlen("prefix5"), addr[7]) == 1);
	assert(nut_set(table, "prefix6", strlen("prefix6"), addr[8]) == 1); // trigger grow
	assert(table->entry_num == 5 && table->table_cap == 8);

	printf("PASS\n");
}