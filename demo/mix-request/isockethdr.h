#include <sys/syscall.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include "nsocket.h"

#define RPI4

#define ETH_P_NIP	0x0909	/* NIP Protocol packet */
#define AF_NNET		0		/* AF_UNSPEC */
#define SOCK_NDP	SOCK_DGRAM

#if __x86_64__
#define SYS_isend	333
#define SYS_irecv	334
#define SYS_touch	335
#define SYS_request	336
#define SYS_publish	337
#define SYS_unpublish	338
#define SYS_pump	339
#define SYS_unpump	340
#define SYS_cast	341
#define SYS_watch	342
#define SYS_unwatch	343
#define SYS_status	344
#elif __arm__
#ifdef RPI4
#define SYS_isend	400
#define SYS_irecv	401
#define SYS_touch	402
#define SYS_request	403
#define SYS_publish	404
#define SYS_unpublish	405
#define SYS_pump	406
#define SYS_unpump	407
#define SYS_cast	408
#define SYS_watch	409
#define SYS_unwatch	410
#define SYS_status	411
#endif /* rpi vernum */
#endif /* arch */


enum {
	NIPPROTO_ALL = 1,
#define NIPPROTO_ALL		NIPPROTO_ALL
	NIPPROTO_ILISTEN = 2,
#define NIPPROTO_ILISTEN	NIPPROTO_ILISTEN
	NIPPROTO_DLISTEN = 3,
#define NIPPROTO_DLISTEN	NIPPROTO_DLISTEN
	NIPPROTO_MAX
};

size_t _get_nlen(char *name, char *bound)
{
	size_t i;
	for (i = 0; name + i < bound; i++) {
		if (name[i] == '\n')
			break;
	}
	return i;
}

void print_emsg(void)
{
	char* emsg = strerror(errno);
	printf("[!] Emsg: %s\n", emsg);
}

int isend(int fd, struct isockbuf *ibuf, size_t len, unsigned int flags)
{
	return syscall(SYS_isend, fd, ibuf, len, flags);
}

int irecv(int fd, struct isockbuf *ibuf, size_t len, unsigned int flags)
{
	return syscall(SYS_irecv, fd, ibuf, len, flags);
}

int touch(int fd, struct touch_info *tbuf, size_t buf_size, 
	  unsigned int flags)
{
	return syscall(SYS_touch, fd, tbuf, buf_size, flags);
}

int request(int fd, struct touch_info *buf, size_t len)
{
	return syscall(SYS_request, fd, buf, len);
}

int publish(int fd, struct isockbuf *buf, size_t len)
{
	return syscall(SYS_publish, fd, buf, len);
}

int unpublish(int fd, char *name, size_t nlen)
{
	return syscall(SYS_unpublish, fd, name, nlen);
}

int pump(int fd, struct isockbuf *buf, size_t len, unsigned int seg_limit,
	 unsigned int flags)
{
	return syscall(SYS_pump, fd, buf, len, seg_limit, flags);
}

int unpump(int fd, char *name, size_t nlen)
{
	return syscall(SYS_unpump, fd, name, nlen);
}

int cast(int fd, struct isockbuf *buf, size_t len, __u32 life_time)
{
	return syscall(SYS_cast, fd, buf, len, life_time);
}

int watch(int fd, char *name, size_t nlen)
{
	return syscall(SYS_watch, fd, name, nlen);
}

int unwatch(int fd, char *name, size_t nlen)
{
	return syscall(SYS_unwatch, fd, name, nlen);
}

int status(int fd, struct status_info *stat_buf, size_t buf_len, 
	   unsigned int flags)
{
	return syscall(SYS_status, fd, stat_buf, buf_len, flags);
}