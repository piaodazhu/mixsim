#ifndef NSOCKET_H
#define NSOCKET_H

#include <linux/types.h>
#include <linux/in.h>	/* Need by sockaddr_in */
#include <stdbool.h>

/* Note: the max size is 128 Bytes */
struct nsockaddr {
	unsigned short int	family;
	__u16 			enable_mask;

#define BIND_PREFIX		0x0001
	char			*prefix;
	size_t			plen;

#define BIND_FACEID		0x0002
	__u16			face_id;

//#define XXNET_ENABLE		0x0003
};
#define MAX_ENABLE		BIND_INETADDR


#define _COMMON_TOUCH_INFO_TYPE		0x01
#define __PIN_LEN__			4	/* PIN_LEN defined in pub.h */
struct cmn_tinfo {
	/* Common TouchInfo */
	__u16			base_name_len;
	char			pin[__PIN_LEN__];
	
	/* Common SegInfo */
	__u32			min_seg_num;
	__u32			max_seg_num;
	
	__u16			mtu;
	__u16			pkt_len;
};

#define _GENERAL_TOUCH_INFO_TYPE	0x06
#define GEN_TINFO_FIXED_LEN		(sizeof(__u16)+sizeof(__u64))
struct gen_tinfo {
	__u16			tbuf_len;
	char			__first;
};

struct nack_tinfo {
	char			__first;
};

struct touch_info {
	__u16 			touchinfo_type;
	char			*name;
	__u16			nlen;
	bool			nack;
	__u16			nack_clen;
	__u32			__realsize;
	union {
		struct cmn_tinfo	cmn_tinfo;
		struct gen_tinfo	gen_tinfo;
		struct nack_tinfo	nack_tinfo;
	};
};

/* Helpers for user */
static inline __u16 cmn_tinfo_basenamelen(struct touch_info *tinfo)
{
	return tinfo->cmn_tinfo.base_name_len;
}

static inline char *cmn_tinfo_pin(struct touch_info *tinfo)
{
	return tinfo->cmn_tinfo.pin;
}

static inline __u32 cmn_tinfo_minsegnum(struct touch_info *tinfo)
{
	return tinfo->cmn_tinfo.min_seg_num;
}

static inline __u32 cmn_tinfo_maxsegnum(struct touch_info *tinfo)
{
	return tinfo->cmn_tinfo.max_seg_num;
}

static inline __u32 cmn_tinfo_mtu(struct touch_info *tinfo)
{
	return tinfo->cmn_tinfo.mtu;
}

static inline __u16 cmn_tinfo_pktlen(struct touch_info *tinfo)
{
	return tinfo->cmn_tinfo.pkt_len;
}

static inline __u16 gen_tinfo_len(struct touch_info *tinfo)
{
	return tinfo->gen_tinfo.tbuf_len;
}

static inline char *gen_tinfo_data(struct touch_info *tinfo)
{
	return &(tinfo->gen_tinfo.__first);
}

static inline char *nack_tinfo_data(struct touch_info *tinfo)
{
	return &(tinfo->nack_tinfo.__first);
}

struct status_info {
	__u32			watch_id;
	bool			satisfied;

	size_t			nlen;
	char			*name_buf;
};

struct stat_info {
	__u32			watch_id;
	bool			satisfied;

	size_t			nlen;
	struct msghdr 		*msg_p;
};

#define TYPE_INTEREST			0x05
#define TYPE_DATA			0x06

#define I_ISTOUCH			0x01
#define I_CANBEPREFIX			0x02
#define I_MUSTBEFRESH			0x04

#define D_BLOB				0x00
#define D_NACK				0x01
#define D_LINK				0x02
#define D_KEY				0x04

struct isockbuf {
	unsigned int		type;
	char			*name;
	__u16			nlen;
	char			*data;
	__u64			dlen;

	union {
		struct {
			__u8	i_flags;
			__u32	i_life_time;
			__u64	exp_data_digest;
			__u32	app_params_len;
			void	*app_params;
		};
		struct {
			__u8	d_type;
			__u32	fresh_period;
			__u32	sig_len;
			void	*sig;
		};
	};
};

struct isockbuf_k {
	unsigned int			type;
	struct msghdr			*nmsg_p;
	__u16				nlen;
	struct msghdr			*dmsg_p;
	__u64				dlen;

	union {
		struct {
			__u8		i_flags;
			__u32		i_life_time;
			__u64		exp_data_digest;
			__u32		app_params_len;
			struct msghdr	*app_msg_p;
		};
		struct {
			__u8		d_type;
			__u32		fresh_period;
			__u32		sig_len;
			struct msghdr	*sig_msg_p;
		};
	};
};

struct __isockbuf_k {
	struct isockbuf_k	*isk_buf;
	void 			*u_name;
	void 			*u_data;
	union {
		void 		*u_app;
		void 		*u_sig;
	};
};

#ifndef SIOCADDRT
#define SIOCADDRT	0x890B		/* ref to sockios.h, add fib entry */
#endif
struct fib_entry_in {
	char	*prefix;	/* net_device name */
	__u16	len;		/* length of @prefix */
	
	__u8	face_id;	/* destination face_id */
};

#endif /* NSOCKET_H */