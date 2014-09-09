#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "syslog.h"
#include "getcfg_v2.h"
#include "tcpconn.h"
#include "set_keepalive.h"
#include "xdr_pkt.h"
#include "xdr_sendto_merge.h"
#include "xdr_handler.h"
#include "xdr_proc_u.h"
#include "xdr_proc_s.h"

#define KEEPALIVE_IDEL	30	/* start keeplives after period(seconds) */
#define KEEPALIVE_CNT	5	/* number of keepalives before death */
#define KEEPALIVE_INTV	3	/* interval between keepalives */
#define INVALID_SOCKET	-1
#define LOCK(mtx)	pthread_mutex_lock(&mtx)
#define UNLOCK(mtx)	pthread_mutex_unlock(&mtx)

#define FIX_FMT \
	"------------- merge packet -------------\n" \
	"  pkt_len   : %d\n" \
	"  imsi      : %02X%02X%02X%02X%02X%02X%02X%02X\n" \
	"  imei      : %02X%02X%02X%02X%02X%02X%02X%02X\n" \
	"  msisdn    : %02X%02X%02X%02X%02X%02X%02X%02X" \
	              "%02X%02X%02X%02X%02X%02X%02X%02X\n" \
	"  interface : %d\n" \
	"  xdr_id    : %02X%02X%02X%02X%02X%02X%02X%02X" \
	              "%02X%02X%02X%02X%02X%02X%02X%02X\n" \
	"  procedure : %d %d %d\n"
#define FIX_VAL(pktlen, imsi, imei, isdn, interface, id, \
		proc_type, proc_status, proc_cause) \
	pktlen, \
	imsi[0],imsi[1],imsi[2],imsi[3],imsi[4],imsi[5],imsi[6],imsi[7], \
	imei[0],imei[1],imei[2],imei[3],imei[4],imei[5],imei[6],imei[7], \
	isdn[0],isdn[1],isdn[2],isdn[3],isdn[4],isdn[5],isdn[6],isdn[7], \
	isdn[8],isdn[9],isdn[10],isdn[11],isdn[12],isdn[13],isdn[14],isdn[15], \
	interface, \
	id[0],id[1],id[2],id[3],id[4],id[5],id[6],id[7], \
	id[8],id[9],id[10],id[11],id[12],id[13],id[14],id[15], \
	proc_type, proc_status, proc_cause

#define UU_FMT \
	"  UU info   : %02X%02X %02X %02X%02X%02X %02X%02X%02X%02X\n"
#define UU_VAL(grp_id, mme_code, enb_id, cell_id) \
	grp_id[0], grp_id[1], \
	mme_code, \
	enb_id[0], enb_id[1], enb_id[2], \
	cell_id[0], cell_id[1], cell_id[2], cell_id[3]

#define X2_FMT \
	"  X2 info   : %02X%02X %02X %02X%02X%02X %02X%02X%02X%02X" \
	"%02X%02X%02X %02X%02X%02X%02X\n"
#define X2_VAL(grp_id, mme_code, src_enb, src_cell, dst_enb, dst_cell) \
	grp_id[0], grp_id[1], mme_code, \
	src_enb[0], src_enb[1], src_enb[2], \
	src_cell[0], src_cell[1], src_cell[2], src_cell[3], \
	dst_enb[0], dst_enb[1] ,dst_enb[2], \
	dst_cell[0], dst_cell[1], dst_cell[2], dst_cell[3]

#define S1MME_FMT \
	"  S1MME info: %02X %02X%02X %02X %d\n"
#define S1MME_VAL(keyword, grp_id, mme_code, bearer_num) \
	keyword, grp_id[0], grp_id[1], mme_code, bearer_num

#pragma pack(1)
/* User info. All the following field get from XDR header */
struct xdr_merge_user_t {
	byte city[2];
	byte rat;
	byte imsi[8];
	byte imei[8];
	byte msisdn[16];
};
#pragma pack()

#pragma pack(1)
/* single interface info */
struct xdr_merge_single_t {
	byte interface;		/* [XDR header] */
	byte xdr_id[16];	/* [XDR header] */
	byte procedure_type;	/* [XDR payload] */
	byte procedure_start[8];/* [XDR payload] */
	byte procedure_end[8];	/* [XDR payload] */
	byte start_loc_lng[8];
	byte start_loc_lat[8];
	byte end_loc_lng[8];
	byte end_loc_lat[8];
	byte procedure_status;	/* [XDR payload] */
	byte cause;		/* [XDR payload] */
};
#pragma pack()

#pragma pack(1)
struct xdr_merge_uu_t {
	byte mme_group_id[2];
	byte mme_code;
	byte enb_id[3];
	byte cell_id[4];
};
#pragma pack()

#pragma pack(1)
struct xdr_merge_x2_t {
	byte mme_group_id[2];
	byte mme_code;
	byte src_enb_id[3];
	byte src_cell_id[4];
	byte dst_enb_id[3];
	byte dst_cell_id[4];
};
#pragma pack()

#pragma pack(1)
struct xdr_merge_bearer_t {
	byte id;
	byte type;
	byte qci;
	byte status;
	byte enb_gtp_teid[4];
	byte sgw_gtp_teid[4];
};
#pragma pack()

#pragma pack(1)
struct xdr_merge_s1mme_t {
	byte keyword;
	byte mme_group_id[2];
	byte mme_code;
	byte ipv4[4];
	byte ipv6[16];
	byte tac[2];
	byte other_tac[2];

	byte bearer_num;
	byte bearer[sizeof(struct xdr_merge_bearer_t) * XDR_BEARER_NUM_MAX];
};
#pragma pack()

#pragma pack(1)
struct xdr_merge_t {
	byte pkt_len[2];
	struct xdr_merge_user_t user;
	struct xdr_merge_single_t single;

	union {
		struct xdr_merge_uu_t uu;
		struct xdr_merge_x2_t x2;
		struct xdr_merge_s1mme_t s1mme;
	} priv;
};
#pragma pack()

static int handle_mode;
static char server_ip[32] = {0};
static int  server_port = -1;
static int  server_sock = INVALID_SOCKET;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static int xdr_merge_s1mme_fix_len = 
	(long)(&((struct xdr_merge_s1mme_t *)0)->bearer);
static int xdr_merge_fix_len = (long)(&((struct xdr_merge_t *)0)->priv);

static int getcfg()
{
	if (getcfg_v2(CFGFILE, "xdr_merge", "server_ip", server_ip,
				GETCFG_STR) != 0) {
		syslog(LOG_ERR, "xdr_merge server_ip not defined.");
		return -1;
	}
	if (getcfg_v2(CFGFILE, "xdr_merge", "server_port", &server_port,
				GETCFG_INT32) != 0) {
		syslog(LOG_ERR, "xdr_merge server_port not defined.");
		return -1;
	}
	return 0;
}

static void dummy_recv_loop(int sock)
{
	char buff[1024];
	int bytes;

	while (1) {
		bytes = recv(sock, buff, sizeof(buff), 0);
		if (bytes == -1 && errno == EINTR)
			continue;
		if (bytes <= 0)
			break;
	}
}

static void *keepalive_thread(void *arg)
{
	int sock;
	int print_errmsg = 1;

	while (1) {
		sock = tcpconn(server_ip, server_port, 5);
		if (sock == -1) {
			if (print_errmsg == 1) {
				syslog(LOG_ERR, "connect merge module failed.");
				print_errmsg = 0;
			}
			sleep(5);
			continue;
		}
		syslog(LOG_INFO, "connect merge module success.");
		set_keepalive(sock, KEEPALIVE_IDEL, KEEPALIVE_CNT, 
				KEEPALIVE_INTV);

		LOCK(mtx);
		server_sock = sock;
		UNLOCK(mtx);

		dummy_recv_loop(server_sock);

		LOCK(mtx);
		close(server_sock);
		server_sock = INVALID_SOCKET;
		UNLOCK(mtx);
		syslog(LOG_INFO, "disconnect from merge module.");

		print_errmsg = 1;
		sleep(5);
	}

	return NULL;
}

static inline void fill_user(struct xdr_merge_user_t *user,
		byte *city, byte rat, byte *imsi, byte *imei, byte *msisdn)
{
	xdr_copy_bytes(user->city, city, sizeof(user->city));
	user->rat = rat;
	xdr_copy_bytes(user->imsi, imsi, sizeof(user->imsi));
	xdr_copy_bytes(user->imei, imei, sizeof(user->imei));
	xdr_copy_bytes(user->msisdn, msisdn, sizeof(user->msisdn));
}

static inline void fill_single(struct xdr_merge_single_t *single, char *xdr)
{
	struct xdr_hdr_t *hdr = (struct xdr_hdr_t *)xdr;

	single->interface = hdr->interface;
	xdr_copy_bytes(single->xdr_id, hdr->xdr_id, sizeof(single->xdr_id));
	single->procedure_type = get_xdr_procedure_type(xdr);
	xdr_copy_bytes(single->procedure_start, 
			get_xdr_procedure_start(xdr), 
			sizeof(single->procedure_start));
	xdr_copy_bytes(single->procedure_end, 
			get_xdr_procedure_end(xdr),
			sizeof(single->procedure_end));
	single->procedure_status = get_xdr_procedure_status(xdr);
	single->cause = get_xdr_cause(xdr);
}

static inline int fill_uu_priv(struct xdr_merge_uu_t *uu, char *xdr)
{
	struct xdr_pkt_uu_t *src = (struct xdr_pkt_uu_t *)xdr;

	xdr_copy_bytes(uu->mme_group_id, src->mme_group_id, 
			sizeof(uu->mme_group_id));
	uu->mme_code = src->mme_code;
	xdr_copy_bytes(uu->enb_id, src->enb_id, sizeof(uu->enb_id));
	xdr_copy_bytes(uu->cell_id, src->cell_id, sizeof(uu->cell_id));
	return sizeof(struct xdr_merge_uu_t);
}

static inline int fill_x2_priv(struct xdr_merge_x2_t *x2, char *xdr)
{
	struct xdr_pkt_x2_t *src = (struct xdr_pkt_x2_t *)xdr;

	xdr_copy_bytes(x2->mme_group_id, src->mme_group_id,
			sizeof(x2->mme_group_id));
	x2->mme_code = src->mme_code;
	xdr_copy_bytes(x2->src_enb_id, src->src_enb_id,
			sizeof(x2->src_enb_id));
	xdr_copy_bytes(x2->src_cell_id, src->src_cell_id,
			sizeof(x2->src_cell_id));
	xdr_copy_bytes(x2->dst_enb_id, src->dst_enb_id,
			sizeof(x2->dst_enb_id));
	xdr_copy_bytes(x2->dst_cell_id, src->dst_cell_id,
			sizeof(x2->dst_cell_id));
	return sizeof(struct xdr_merge_x2_t);
}

static inline int fill_s1_mme_priv(struct xdr_merge_s1mme_t *s1mme, char *xdr)
{
	struct xdr_pkt_s1_mme_t *src = (struct xdr_pkt_s1_mme_t *)xdr;
	int bearer_len = 0;

	s1mme->keyword = src->keyword;
	xdr_copy_bytes(s1mme->mme_group_id, src->mme_group_id,
			sizeof(s1mme->mme_group_id));
	s1mme->mme_code = src->mme_code;
	xdr_copy_bytes(s1mme->ipv4, src->ipv4, sizeof(s1mme->ipv4));
	xdr_copy_bytes(s1mme->ipv6, src->ipv6, sizeof(s1mme->ipv6));
	xdr_copy_bytes(s1mme->tac, src->tac, sizeof(s1mme->tac));
	xdr_copy_bytes(s1mme->other_tac, src->other_tac,
			sizeof(s1mme->other_tac));
	s1mme->bearer_num = src->bearer_num;

	if (s1mme->bearer_num > 0 &&
			s1mme->bearer_num <= XDR_BEARER_NUM_MAX) {
		bearer_len = sizeof(struct xdr_merge_bearer_t) * 
			s1mme->bearer_num;
		xdr_copy_bytes(s1mme->bearer, &src->bearer_num + 1, bearer_len);
	}

	return xdr_merge_s1mme_fix_len + bearer_len;
}

static inline int fill_priv(void *priv, char *xdr)
{
	int type = get_xdr_interface((struct xdr_hdr_t *)xdr);

	if (type == XDR_TYPE_UU)
		return fill_uu_priv((struct xdr_merge_uu_t *)priv, xdr);
	else if (type == XDR_TYPE_X2)
		return fill_x2_priv((struct xdr_merge_x2_t *)priv, xdr);
	else if (type == XDR_TYPE_S1_MME)
		return fill_s1_mme_priv((struct xdr_merge_s1mme_t *)priv, xdr);
	return 0;
}

static inline void fill_pkt_len(byte *pkt_len, int val)
{
	ENCODE_16(pkt_len, val);
}

static int send_xdr_u(void *val)
{
	struct map_u_val_t *pval = (struct map_u_val_t *)val;
	list<struct xdr_pkt_t>::iterator iter;
	struct xdr_merge_t merge;
	struct xdr_hdr_t *hdr;
	int priv_len;

	printf("prepare send packet to merge module, list size: %d\n", 
			(int)pval->lst.size());

	for (iter = pval->lst.begin(); iter != pval->lst.end(); iter++) {
		hdr = (struct xdr_hdr_t *)iter->pkt;
		fill_user(&merge.user, hdr->city, hdr->rat, pval->imsi,
				pval->imei, pval->msisdn);
		fill_single(&merge.single, iter->pkt);
		priv_len = fill_priv(&merge.priv, iter->pkt);
		fill_pkt_len(merge.pkt_len, xdr_merge_fix_len + priv_len);
		send(server_sock, &merge, xdr_merge_fix_len + priv_len, 0);
		printf("send to merge module\n");
	}
	return 0;
}

static int send_xdr_s(void *val)
{
	struct map_s_val_t *pval = (struct map_s_val_t *)val;
	list<struct xdr_pkt_t>::iterator iter;
	struct xdr_merge_t merge;
	struct xdr_hdr_t *hdr;

	printf("prepare send packet to merge module, list size: %d\n", 
			(int)pval->lst.size());

	for (iter = pval->lst.begin(); iter != pval->lst.end(); iter++) {
		hdr = (struct xdr_hdr_t *)iter->pkt;
		fill_user(&merge.user, hdr->city, hdr->rat, hdr->imsi,
				pval->imei, pval->msisdn);
		fill_single(&merge.single, iter->pkt);
		fill_pkt_len(merge.pkt_len, xdr_merge_fix_len);

		send(server_sock, &merge, xdr_merge_fix_len, 0);
		printf("send to merge module\n");
	}
	return 0;
}

static inline void dump_uu(int pkt_len, struct xdr_merge_t *merge)
{
	struct xdr_merge_uu_t *uu = &(merge->priv.uu);

	printf(FIX_FMT UU_FMT,
		FIX_VAL(pkt_len, 
			merge->user.imsi,
			merge->user.imei,
			merge->user.msisdn,
			XDR_TYPE_UU,
			merge->single.xdr_id,
			merge->single.procedure_type,
			merge->single.procedure_status,
			merge->single.cause),
		UU_VAL(uu->mme_group_id,
			uu->mme_code,
			uu->enb_id,
			uu->cell_id));
}

static inline void dump_x2(int pkt_len, struct xdr_merge_t *merge)
{
	struct xdr_merge_x2_t *x2 = &(merge->priv.x2);

	printf(FIX_FMT X2_FMT,
		FIX_VAL(pkt_len, 
			merge->user.imsi,
			merge->user.imei,
			merge->user.msisdn,
			XDR_TYPE_X2,
			merge->single.xdr_id,
			merge->single.procedure_type,
			merge->single.procedure_status,
			merge->single.cause),
		X2_VAL(x2->mme_group_id,
			x2->mme_code,
			x2->src_enb_id,
			x2->src_cell_id,
			x2->dst_enb_id,
			x2->dst_cell_id));
}

static inline void dump_s1mme(int pkt_len, struct xdr_merge_t *merge)
{
	struct xdr_merge_s1mme_t *s1mme = &(merge->priv.s1mme);
	printf(FIX_FMT S1MME_FMT,
		FIX_VAL(pkt_len, 
			merge->user.imsi,
			merge->user.imei,
			merge->user.msisdn,
			XDR_TYPE_S1_MME,
			merge->single.xdr_id,
			merge->single.procedure_type,
			merge->single.procedure_status,
			merge->single.cause),
		S1MME_VAL(s1mme->keyword,
			s1mme->mme_group_id,
			s1mme->mme_code,
			s1mme->bearer_num));
}

static inline void dump_fix(int interface, int pkt_len, 
		struct xdr_merge_t *merge)
{
	printf(FIX_FMT, FIX_VAL(pkt_len, 
			merge->user.imsi,
			merge->user.imei,
			merge->user.msisdn,
			interface,
			merge->single.xdr_id,
			merge->single.procedure_type,
			merge->single.procedure_status,
			merge->single.cause));
}

int xdrs2m_start(int mode)
{
	pthread_t tid;

	handle_mode = mode;

	if (getcfg() != 0)
		return -1;

	if (pthread_create(&tid, NULL, keepalive_thread, NULL) != 0) {
		syslog(LOG_ERR, "create xdrs2m keepalive_thread failed.");
		return -1;
	}
	pthread_detach(tid);
	return 0;
}

int xdrs2m_send(void *val)
{
	int ret = -1;
	LOCK(mtx);

	if (server_sock != INVALID_SOCKET) {
		if (handle_mode == XDR_MODE_U)
			ret = send_xdr_u(val);
		else if (handle_mode == XDR_MODE_S)
			ret = send_xdr_s(val);
	}

	UNLOCK(mtx);
	return ret;
}

void xdrs2m_dump(char *buff)
{
	struct xdr_merge_t *merge = (struct xdr_merge_t *)buff;
	int pkt_len;
	int type;

	DECODE_16(merge->pkt_len, pkt_len);
	DECODE_8(&merge->single.interface, type);

	if (type == XDR_TYPE_UU) {
		dump_uu(pkt_len, merge);
	} else if (type == XDR_TYPE_X2) {
		dump_x2(pkt_len, merge);
	} else if (type == XDR_TYPE_S1_MME) {
		dump_s1mme(pkt_len, merge);
	} else {
		dump_fix(type, pkt_len, merge);
	}
}
