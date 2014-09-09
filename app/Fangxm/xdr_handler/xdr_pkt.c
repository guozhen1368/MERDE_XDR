#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "xdr_pkt.h"

static inline void dump_xdr_uu(char *xdr, char *buff, int buff_size)
{
	struct xdr_pkt_uu_t *uu = (struct xdr_pkt_uu_t *)xdr;
	byte key[XDR_U_KEY_LEN];
	byte *p1 = uu->enb_id;
	byte *p2 = uu->cell_id;

	get_xdr_u_key(key, xdr);

	snprintf(buff, buff_size,
		"\tprocddure_type: %d\n"
		"\tkey           : %02X%02X%02X%02X %02X%02X %02X\n"
		"\tenb & cell    : %02X%02X%02X %02X%02X%02X%02X",
		get_xdr_procedure_type(xdr),
		key[0], key[1], key[2], key[3], key[4], key[5], key[6],
		p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], p2[3]);
}

static inline void dump_xdr_x2(char *xdr, char *buff, int buff_size)
{
	struct xdr_pkt_x2_t *x2 = (struct xdr_pkt_x2_t *)xdr;
	byte key[XDR_U_KEY_LEN];
	byte *p1 = x2->src_enb_id;
	byte *p2 = x2->src_cell_id;
	byte *p3 = x2->dst_enb_id;
	byte *p4 = x2->dst_cell_id;

	get_xdr_u_key(key, xdr);

	snprintf(buff, buff_size,
		"\tprocddure_type: %d\n"
		"\tkey           : %02X%02X%02X%02X %02X%02X %02X\n"
		"\tsrc enb & cell: %02X%02X%02X %02X%02X%02X%02X\n"
		"\tdst enb & cell: %02X%02X%02X %02X%02X%02X%02X",
		get_xdr_procedure_type(xdr),
		key[0], key[1], key[2], key[3], key[4], key[5], key[6],
		p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], p2[3],
		p3[0], p3[1], p3[2], p4[0], p4[1], p4[2], p4[3]);
}

static inline void dump_xdr_s1_mme(char *xdr, char *buff, int buff_size)
{
	byte key[XDR_U_KEY_LEN];

	get_xdr_u_key(key, xdr);

	snprintf(buff, buff_size,
		"\tprocddure_type: %d\n"
		"\tkey           : %02X%02X%02X%02X %02X%02X %02X",
		get_xdr_procedure_type(xdr),
		key[0], key[1], key[2], key[3], key[4], key[5], key[6]);
}

///////////////////////////////////////////////////////////

int xdr_pkt_is_valid(char *xdr, uint32_t bytes)
{
	int type = get_xdr_interface((struct xdr_hdr_t *)xdr);

	if ((type == XDR_TYPE_UU && 
		(bytes >= sizeof(struct xdr_pkt_uu_t) && bytes <= 139)) ||
	    (type == XDR_TYPE_X2 && 
	     	(bytes >= sizeof(struct xdr_pkt_x2_t) && bytes <= 130)) ||
	    (type == XDR_TYPE_S1_MME && 
	     	(bytes >= sizeof(struct xdr_pkt_s1_mme_t) && bytes <= 383 )) ||
	    (type == XDR_TYPE_S6A && bytes == 132) ||
	    (type == XDR_TYPE_SGS && 
	     	(bytes >= sizeof(struct xdr_pkt_sgs_t) && bytes <= 468)))
		return 0;
	return -1;
}

void xdr_copy_bytes(byte *dst, byte *src, int size)
{
	memcpy(dst, src, size);
}

int xdr_is_valid_bytes(byte *buff, int size)
{
	int i;

	for (i = 0; i < size; i++) {
		if (buff[i] != 0xFF)
			return 1;
	}
	return 0;
}

int xdr_dump_hdr(char *xdr, char *buff, int buff_size)
{
	struct xdr_hdr_t *hdr = (struct xdr_hdr_t *)xdr;;
	char city[4 + 1];
	char xdr_id[32 + 1];
	char imsi[16 + 1];
	char imei[16 + 1];
	char msisdn[32 + 1];

	get_xdr_city(hdr, city, sizeof(city));
	get_xdr_id(hdr, xdr_id, sizeof(xdr_id));
	get_xdr_imsi(hdr, imsi, sizeof(imsi));
	get_xdr_imei(hdr, imei, sizeof(imei));
	get_xdr_msisdn(hdr, msisdn, sizeof(msisdn));

	return snprintf(buff, buff_size,
	       "-------------- XDR packet --------------\n"
	       "\tlen           : %d\n"
	       "\tcity          : %s\n"
	       "\tinterface     : %d\n"
	       "\txdrid         : %s\n"
	       "\trat           : %d\n"
	       "\timsi          : %s\n"
	       "\timei          : %s\n"
	       "\tmsisdn        : %s\n",
	       get_xdr_pkt_len(hdr),
	       city,
	       get_xdr_interface(hdr),
	       xdr_id,
	       get_xdr_rat(hdr),
	       imsi,
	       imei,
	       msisdn);
}

void xdr_dump(char *xdr, char *buff, int buff_size)
{
	int type = get_xdr_interface((struct xdr_hdr_t *)xdr);
	int pos;

	pos = xdr_dump_hdr(xdr, buff, buff_size);

	if (type == XDR_TYPE_UU)
		dump_xdr_uu(xdr, &buff[pos], buff_size - pos);
	else if (type == XDR_TYPE_X2)
		dump_xdr_x2(xdr, &buff[pos], buff_size - pos);
	else if (type == XDR_TYPE_S1_MME)
		dump_xdr_s1_mme(xdr, &buff[pos], buff_size - pos);
}

/* The interface to manipulate XDR header */
int get_xdr_pkt_len(struct xdr_hdr_t *hdr)
{
	int val;
	DECODE_16(hdr->pkt_len, val);
	return val;
}

void get_xdr_city(struct xdr_hdr_t *hdr, char *str, int size)
{
	snprintf(str, size, "%02X%02X", hdr->city[0], hdr->city[1]);
}

int get_xdr_interface(struct xdr_hdr_t *hdr)
{
	int val;
	DECODE_8(&hdr->interface, val);
	return val;
}

void get_xdr_id(struct xdr_hdr_t *hdr, char *str, int size)
{
	byte *p = hdr->xdr_id;
	snprintf(str, size, "%02X%02X%02X%02X%02X%02X%02X%02X"
			"%02X%02X%02X%02X%02X%02X%02X%02X",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
}

int get_xdr_rat(struct xdr_hdr_t *hdr)
{
	int val;
	DECODE_8(&hdr->rat, val);
	return val;
}

void get_xdr_imsi(struct xdr_hdr_t *hdr, char *str, int size)
{
	byte *p = hdr->imsi;
	snprintf(str, size, "%02X%02X%02X%02X%02X%02X%02X%02X",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

void get_xdr_imei(struct xdr_hdr_t *hdr, char *str, int size)
{
	byte *p = hdr->imei;
	snprintf(str, size, "%02X%02X%02X%02X%02X%02X%02X%02X",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

void get_xdr_msisdn(struct xdr_hdr_t *hdr, char *str, int size)
{
	byte *p = hdr->msisdn;
	snprintf(str, size, "%02X%02X%02X%02X%02X%02X%02X%02X"
			"%02X%02X%02X%02X%02X%02X%02X%02X",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
}

///////////////////////////////////////////////////////////
/* The interface to manipulate fileds in XDR packet */

int get_xdr_u_key(byte *key, char *xdr)
{
	int type = get_xdr_interface((struct xdr_hdr_t *)xdr);

	if (type == XDR_TYPE_UU) {
		memcpy(key, ((struct xdr_pkt_uu_t *)xdr)->mme_ue_s1ap_id,
				XDR_U_KEY_LEN);
	} else if (type == XDR_TYPE_X2) {
		memcpy(key, ((struct xdr_pkt_x2_t *)xdr)->mme_ue_s1ap_id, 
				XDR_U_KEY_LEN);
	} else if (type == XDR_TYPE_S1_MME) {
		memcpy(key, ((struct xdr_pkt_s1_mme_t *)xdr)->mme_ue_s1ap_id, 
				XDR_U_KEY_LEN);
	} else {
		return -1;
	}
	return 0;
}

int get_xdr_s_key(byte *key, char *xdr)
{
	struct xdr_hdr_t *hdr = (struct xdr_hdr_t *)xdr;
	memcpy(key, hdr->imsi, sizeof(hdr->imsi));
	return 0;
}

void get_xdr_src_enb_key(char *dst, char *xdr)
{
	struct xdr_pkt_x2_t *x2 = (struct xdr_pkt_x2_t *)xdr;
	memcpy(dst,     x2->mme_ue_s1ap_id, 4);
	memcpy(&dst[4], x2->src_enb_id,     3);
	memcpy(&dst[7], x2->src_cell_id,    4);
}

void get_xdr_dst_enb_key(char *dst, char *xdr)
{
	struct xdr_pkt_x2_t *x2 = (struct xdr_pkt_x2_t *)xdr;
	memcpy(dst,     x2->mme_ue_s1ap_id, 4);
	memcpy(&dst[4], x2->dst_enb_id,     3);
	memcpy(&dst[7], x2->dst_cell_id,    4);
}

byte get_xdr_procedure_type(char *xdr)
{
	return ((struct xdr_pkt_uu_t *)xdr)->procedure_type;
}

byte get_xdr_procedure_status(char *xdr)
{
	int type = get_xdr_interface((struct xdr_hdr_t *)xdr);
	if (type == XDR_TYPE_UU)
		return ((struct xdr_pkt_uu_t *)xdr)->procedure_status;
	else
		return ((struct xdr_pkt_x2_t *)xdr)->procedure_status;
}

byte *get_xdr_procedure_start(char *xdr)
{
	return ((struct xdr_pkt_uu_t *)xdr)->procedure_start;
}

byte *get_xdr_procedure_end(char *xdr)
{
	return ((struct xdr_pkt_uu_t *)xdr)->procedure_end;
}

byte get_xdr_cause(char *xdr)
{
	int type = get_xdr_interface((struct xdr_hdr_t *)xdr);
	struct xdr_pkt_sgs_t *sgs;

	if (type == XDR_TYPE_X2)
		return ((struct xdr_pkt_x2_t *)xdr)->failure_cause[1];
	else if (type == XDR_TYPE_S1_MME)
		return ((struct xdr_pkt_s1_mme_t *)xdr)->cause[1];
	else if (type == XDR_TYPE_S6A)
		return ((struct xdr_pkt_s6a_t *)xdr)->cause[1];
	else if (type == XDR_TYPE_SGS) {
		sgs = (struct xdr_pkt_sgs_t *)xdr;
		if (sgs->sgs_cause != 0xF)
			return sgs->sgs_cause;
		if (sgs->reject_cause != 0xF)
			return sgs->reject_cause;
		if (sgs->cp_cause)
			return sgs->cp_cause;
		if (sgs->rp_cause)
			return sgs->rp_cause;
	}
	return 0xF;
}
