#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "name_udp_table.h"

void
nut_grow(nut* table);

int
nut_entry_namematch(nut_entry *entry, char *name, int nlen);

int
nut_entry_fdmatch(nut_entry *entry, struct sockaddr_in addr);

nut*
nut_create(int init_size) {
	if (init_size <= 0 || init_size >= 32) {
		init_size = 32;
	}
	nut* tab = (nut*)malloc(sizeof(nut));
	tab->entry_num = 0;
	tab->table_cap = init_size;
	tab->entries = (nut_entry*)malloc(sizeof(nut_entry) * init_size);
	return tab;
}

int
nut_set(nut* table, char *name, int nlen, struct sockaddr_in addr) {
	int first_deleted = -1;
	int cur_idx;
	for (cur_idx = 0; cur_idx < (int)(table->entry_num); cur_idx++) {
		nut_entry *entry = &table->entries[cur_idx];
		if (entry->deleted == 1) {
			if (first_deleted == -1)
				first_deleted = cur_idx;
			continue;
		}
		if (nut_entry_namematch(entry, name, nlen) > 0) {
			// match name
			if (nut_entry_fdmatch(entry, addr) > 0) {
				// already found. do nothing
				return 0;
			} else {
				// add to addrs
				if (entry->addr_num == MAX_ADDR_PERNAME) {
					return -1;
				}
				entry->addr[entry->addr_num] = addr;
				entry->addr_num++;
				return 1;
			}
		}
	}
	
	// should add a new entry
	if (first_deleted == -1) { // full
		if (cur_idx == (int)(table->table_cap)) {
			// grow up
			nut_grow(table);
		}
		first_deleted = cur_idx;
		table->entry_num++;
	}
	nut_entry *entry = &table->entries[first_deleted];
	entry->deleted = 0;
	entry->addr_num = 1;
	entry->addr[0] = addr;
	if (nlen > MAX_NAMELEN) {
		name += (nlen - MAX_NAMELEN); // move start addr to here
		nlen = MAX_NAMELEN; // truncate it
	}
	entry->name_len = nlen;
	memcpy(entry->name, name, nlen);

	return 1;
}

nut_entry*
nut_get(nut *table, char *name, int nlen) {
	int cur_idx;
	for (cur_idx = 0; cur_idx < (int)(table->entry_num); cur_idx++) {
		nut_entry *entry = &table->entries[cur_idx];
		if (entry->deleted == 1) {
			continue;
		}
		if (nut_entry_namematch(entry, name, nlen) > 0) {
			return entry;
		}
	}
	return NULL;
}

void
nut_del(nut_entry *entry) {
	entry->deleted = 1;
}

void
nut_grow(nut* table) {
	int target_cap;
	if (table->table_cap < 256) {
		// x 2 
		target_cap = table->table_cap * 2;
	} else {
		// x 1.25
		target_cap = table->table_cap + table->table_cap/4;
	}

	// move entries to new buffer
	nut_entry* target_buf = (nut_entry*)malloc(sizeof(nut_entry) * target_cap);
	memcpy((void*)target_buf, (void*)table->entries, sizeof(nut_entry) * table->entry_num);
	// free old buffer
	free(table->entries);
	table->entries = target_buf;
	table->table_cap = target_cap;
}

int
nut_entry_namematch(nut_entry *entry, char *name, int nlen) {
	if (nlen > MAX_NAMELEN) {
		name += (nlen - MAX_NAMELEN); // move start addr to here
		nlen = MAX_NAMELEN; // truncate it
	}
	if (nlen != entry->name_len) {
		return 0;
	}
	int idx;
	for (idx = nlen - 1; idx >= 0; idx--) { // reverse cmp
		if (name[idx] != entry->name[idx]) {
			return 0;
		}
	}
	return 1;
}

int
nut_entry_fdmatch(nut_entry *entry, struct sockaddr_in addr) {
	int idx;
	for (idx = 0; idx < entry->addr_num; idx++) {
		if (entry->addr[idx].sin_addr.s_addr == addr.sin_addr.s_addr 
		&& entry->addr[idx].sin_port == addr.sin_port) {
			return 1;
		}
	}
	return 0;
}
