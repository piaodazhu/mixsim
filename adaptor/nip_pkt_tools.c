#include <linux/errno.h>
#include "tlv_u.h"
#include "nsocket.h"
#include "encap_pkt_buf.h"
#include "decap_pkt_buf.h"
#include "nip_pkt_tools.h"

/*
 *	return pkt type or err code
 */
int get_pkt_type(const char *buf, const size_t len)
{
	int err;
	struct tlv_t _tlv;
	struct tlv_t *tlv = &_tlv;
	
	err = tlv_decode_usr(buf, tlv, len);
	return err ? err : tlv_type(tlv);
}

/*
 *	return name_len or err code
 *	the pointer of the name is buf + offset (when return value > 0)
 */
int get_pkt_name(const char *buf, const size_t len, size_t *offset)
{
	int err;
	struct tlv_t _tlv;
	struct tlv_t *tlv = &_tlv;
	char *tail, *_buf = buf;
	
	err = tlv_decode_usr(buf, tlv, len);
	if (err || (tlv_type(tlv) != TYPE_INTEREST && tlv_type(tlv) != TYPE_DATA))
		return err;
	
	buf = tlv_value(tlv);
	tail = buf + tlv_len(tlv);
	while (buf < tail) {
		if (tlv_decode_usr(buf, tlv, len) < 0) {
			return -EINVAL;
		}
		switch (tlv_type(tlv)) {
		case TYPE_NAME:
			*offset = (char *)tlv_value(tlv) - _buf;
			return tlv_len(tlv);
		default:
			break;
		}
		buf = tlv_value(tlv) + tlv_len(tlv);
	}
	return -EINVAL;
}

/*
 *	return TLV Interest buffer, @buf_len is the length of the buffer
 *	return NULL when failed
 */
size_t encap_interest(char *name, __u64 nlen, __u64 _interestLifetime, 
			   __u8 _hop_limit, __u32 _nonce, __u8 flags, 
			   unsigned char* buf, size_t buf_len)
{
	return encap_interest_buf(name, nlen, _interestLifetime, _hop_limit, 
			       _nonce, flags, buf, buf_len);
}

/*
 *	return TLV Data buffer, @buf_len is the length of the buffer
 *	return NULL when failed
 */
size_t encap_data(char *name, __u64 nlen, char *content, __u64 clen, 
			   unsigned char* buf, size_t buf_len)
{
	return encap_data_buf(name, nlen, content, clen, buf, buf_len);
}

/*
 *	return TLV Common Touch-Data buffer, @buf_len is the length of the buffer
 *	return NULL when failed
 */
size_t encap_tinfo_content(__u64 bname_len, char *pin, __u8 pin_len, 
					__u32 max_segnum, unsigned char *buf, size_t buf_len)
{
	return encap_tinfo_content_buf(bname_len, pin, pin_len, max_segnum, 
					buf, buf_len);
}

/*
 *	@tbuf is user defined content of touch-Data
 *	return TLV General Touch-Data buffer, @buf_len is the length of the buffer
 *	return NULL when failed
 */
size_t encap_gen_tinfo_content(__u64 tbuf_len, unsigned char *tbuf, 
					    unsigned char* buf, size_t buf_len)
{
	return encap_gen_tinfo_content_buf(tbuf_len, tbuf, buf, buf_len);
}

/*
 *	return TLV Interest buffer, @buf_len is the length of the buffer
 *	return NULL when failed
 */
int decap_pkt(unsigned char *buf, size_t buf_len, 
		 struct isockbuf *ibuf, size_t ibuf_len)
{
	struct tlv_t _tlv;
	struct tlv_t *tlv = &_tlv;

	bool can_be_prefix, must_be_fresh;
	__u32 nonce;
	__u8 hop_limit;

	int res = -EINVAL;
	if (tlv_decode_usr(buf, tlv, buf_len) < 0) {
		printf("[!] tlv_decode(<nip_rcv>) < 0, DROP \n");
		goto out;
	}
	ibuf->type = tlv_type(tlv);
	switch (ibuf->type) {
	case TYPE_INTEREST:
		res = decap_interest_buf(tlv, &ibuf->name, &ibuf->nlen, 
					 &can_be_prefix, &must_be_fresh, 
					 &nonce, &hop_limit, &ibuf->i_life_time);
		// ibuf->data = NULL;
		ibuf->dlen = 0;
		if (can_be_prefix)
			ibuf->i_flags |= I_CANBEPREFIX;
		if (must_be_fresh)
			ibuf->i_flags |= I_MUSTBEFRESH;
		break;
	case TYPE_DATA:
		res = decap_data_buf(tlv, &ibuf->name, &ibuf->nlen, 
				     &ibuf->data, &ibuf->dlen, &ibuf->d_type, 
				     &ibuf->fresh_period);
		break;
	default:
		printf("[!] Unknown packet type: %u \n", ibuf->type);
		break;
	}
out:	return res;
}