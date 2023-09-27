CC=gcc
CFLAGS=-std=gnu99 -O3 -Wall -Wextra -g
LDFLAGS=-g
LOADLIBS=-lpthread

all : adaptor liblfq.so.1.0.0
adaptor: adaptor.o lfq.o name_udp_table.o nip_adaptor.o udp_adaptor.o
	gcc $(CFLAGS) $(LDFLAGS) $^ -o adaptor -ldyco -I.

liblfq.so.1.0.0: lfq.c lfq.h cross-platform.h
	gcc $(CFLAGS) $(CPPFLAGS) -c lfq.c   # -fno-pie for static linking?
	ar rcs liblfq.a lfq.o
	gcc $(CFLAGS) $(CPPFLAGS) -fPIC -c lfq.c
	gcc $(LDFLAGS) -shared -o liblfq.so.1.0.0 lfq.o

adaptor.o:
lfq.o:
name_udp_table.o:
nip_adaptor.o:
udp_adaptor.o:

clean:
	rm -rf *.o liblfq.so.1.0.0 liblfq.a adaptor
