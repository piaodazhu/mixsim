#ifndef NIP_PKT_TOOLS_H
#define NIP_PKT_TOOLS_H
#include <sys/types.h>
#include <linux/types.h>
#include "nsocket.h"

int get_pkt_type(const char *buf, const size_t len);
int get_pkt_name(const char *buf, const size_t len, size_t *offset);

size_t encap_interest(char *name, __u64 nlen, __u64 _interestLifetime, 
			   __u8 _hop_limit, __u32 _nonce, __u8 flags, 
			   unsigned char* buf, size_t buf_len);
size_t encap_data(char *name, __u64 nlen, char *content, __u64 clen, 
			   unsigned char* buf, size_t buf_len);
size_t encap_tinfo_content(__u64 bname_len, char *pin, __u8 pin_len, 
					__u32 max_segnum, unsigned char *buf, size_t buf_len);
size_t encap_gen_tinfo_content(__u64 tbuf_len, unsigned char *tbuf, 
					    unsigned char* buf, size_t buf_len);

int decap_pkt(unsigned char *buf, size_t buf_len, 
	      struct isockbuf *ibuf, size_t ibuf_len);

#endif /* NIP_PKT_TOOLS_H */