#ifndef _XDR_PKT_H_
#define _XDR_PKT_H_

#include <stdint.h>

/* max length of one XDR packet */
#define XDR_LEN_MAX		480

/* max bearer number (0 ~ 15) */
#define XDR_BEARER_NUM_MAX	16

/* mme_ue_s1ap_id[4], mme_group_id[2], mme_code[1] */
#define XDR_U_KEY_LEN		7

/* imsi[8] */
#define XDR_S_KEY_LEN		8

/* XDR packet type */
#define XDR_TYPE_UU		1
#define XDR_TYPE_X2		2
#define XDR_TYPE_UE_MR		3
#define XDR_TYPE_CELL_MR	4
#define XDR_TYPE_S1_MME		5
#define XDR_TYPE_S6A		6
#define XDR_TYPE_S11		7
#define XDR_TYPE_S10		8
#define XDR_TYPE_SGS		9
#define XDR_TYPE_S5_S8		10
#define XDR_TYPE_S1_U		11
#define XDR_TYPE_GN_C		12

#ifndef byte
#define byte	unsigned char
#endif

#define ENCODE_8(ptr, val) { \
	byte *coding_ptr = (byte *)(ptr); \
	*coding_ptr = (val) & 0xff; \
}

#define DECODE_8(ptr, val) { \
	byte *decoding_ptr = (byte *)(ptr); \
	(val) = *decoding_ptr; \
}

#define ENCODE_16(ptr, val) { \
	byte *coding_ptr = (byte *)(ptr); \
	*coding_ptr++ = ((val) >> 8) & 0xff; \
	*coding_ptr = (val) & 0xff; \
}

#define DECODE_16(ptr, val) { \
	byte *decoding_ptr = (byte *)(ptr); \
	(val) = (*decoding_ptr << 8) | *(decoding_ptr + 1); \
}

#define ENCODE_24(ptr, val) { \
	byte *coding_ptr = (byte *)(ptr); \
	*coding_ptr++ = ((val) >> 16) & 0xff; \
	*coding_ptr++ = ((val) >> 8) & 0xff; \
	*coding_ptr = (val) & 0xff; \
}

#define DECODE_24(ptr, val) { \
	byte *decoding_ptr = (byte *)(ptr); \
	(val) = (*decoding_ptr << 16) | \
	(*(decoding_ptr + 1) << 8) | *(decoding_ptr + 2); \
}

#define ENCODE_32(ptr, val) { \
	byte *coding_ptr = (byte *)(ptr); \
	*coding_ptr++ = ((val) >> 24) & 0xff; \
	*coding_ptr++ = ((val) >> 16) & 0xff; \
	*coding_ptr++ = ((val) >> 8) & 0xff; \
	*coding_ptr = (val) & 0xff; \
}

#define DECODE_32(ptr, val) { \
	byte *decoding_ptr = (byte *)(ptr); \
	(val) = (*decoding_ptr << 24) | \
	(*(decoding_ptr + 1) << 16) | \
	(*(decoding_ptr + 2) << 8) | *(decoding_ptr + 3); \
}

#define ENCODE_64(ptr, val) { \
	byte *coding_ptr = (byte *)(ptr); \
	*coding_ptr++ = ((val) >> 56) & 0xff; \
	*coding_ptr++ = ((val) >> 48) & 0xff; \
	*coding_ptr++ = ((val) >> 40) & 0xff; \
	*coding_ptr++ = ((val) >> 32) & 0xff; \
	*coding_ptr++ = ((val) >> 24) & 0xff; \
	*coding_ptr++ = ((val) >> 16) & 0xff; \
	*coding_ptr++ = ((val) >> 8) & 0xff; \
	*coding_ptr = (val) & 0xff; \
}

#define DECODE_64(ptr, val) { \
	byte *decoding_ptr = (byte *)(ptr); \
	(val) = ((uint64_t)*decoding_ptr << 56) | \
	((uint64_t)*(decoding_ptr + 1) << 48) | \
	((uint64_t)*(decoding_ptr + 2) << 40) | \
	((uint64_t)*(decoding_ptr + 3) << 32) | \
	((uint64_t)*(decoding_ptr + 4) << 24) | \
	((uint64_t)*(decoding_ptr + 5) << 16) | \
	((uint64_t)*(decoding_ptr + 6) << 8) | \
	(uint64_t)*(decoding_ptr + 7); \
}

#pragma pack(1)
struct xdr_pkt_t {
	int pkt_len;
	char pkt[XDR_LEN_MAX];
};
#pragma pack()

/* XDR packet header */
#pragma pack(1)
struct xdr_hdr_t {
	byte pkt_len[2];
	byte city[2];
	byte interface;
	byte xdr_id[16];
	byte rat;
	byte imsi[8];
	byte imei[8];
	byte msisdn[16];
};
#pragma pack()

/* Uu packet */
#pragma pack(1)
struct xdr_pkt_uu_t {
	struct xdr_hdr_t hdr;

	byte procedure_type;
	byte procedure_start[8];
	byte procedure_end[8];
	byte IGNORE_1;
	byte procedure_status;
	byte IGNORE_2[3];
	byte enb_id[3];
	byte cell_id[4];
	byte IGNORE_3[11];
	byte mme_ue_s1ap_id[4];
	byte mme_group_id[2];
	byte mme_code;

	/* ignore left */
};
#pragma pack()

/* X2 packet */
#pragma pack(1)
struct xdr_pkt_x2_t {
	struct xdr_hdr_t hdr;

	byte procedure_type;
	byte procedure_start[8];
	byte procedure_end[8];
	byte procedure_status;
	byte src_cell_id[4];
	byte dst_cell_id[4];
	byte src_enb_id[3];
	byte dst_enb_id[3];
	byte mme_ue_s1ap_id[4];
	byte mme_group_id[2];
	byte mme_code;
	byte IGNORE[2];
	byte failure_cause[2];

	/* ignore left */
};
#pragma pack()

/* S1-MME packet */
#pragma pack(1)
struct xdr_pkt_s1_mme_t {
	struct xdr_hdr_t hdr;

	byte procedure_type;
	byte procedure_start[8];
	byte procedure_end[8];
	byte procedure_status;
	byte cause[2];
	byte keyword;
	byte mme_ue_s1ap_id[4];
	byte mme_group_id[2];
	byte mme_code;
	byte IGNORE_1[8];
	byte ipv4[4];
	byte ipv6[16];
	byte IGNORE_2[36];
	byte tac[2];
	byte IGNORE_3[4];
	byte other_tac[2];
	byte IGNORE_4[36];
	byte bearer_num;

	/* ignore left */
};
#pragma pack()

/* S6a packet */
#pragma pack(1)
struct xdr_pkt_s6a_t {
	struct xdr_hdr_t hdr;

	byte procedure_type;
	byte procedure_start[8];
	byte procedure_end[8];
	byte procedure_status;
	byte cause[2];

	/* ignore left */
};
#pragma pack()

/* SGs packet */
#pragma pack(1)
struct xdr_pkt_sgs_t {
	struct xdr_hdr_t hdr;

	byte procedure_type;
	byte procedure_start[8];
	byte procedure_end[8];
	byte procedure_status;
	byte sgs_cause;
	byte reject_cause;
	byte cp_cause;
	byte rp_cause;

	/* ignore left */
};
#pragma pack()

/* get XDR packet content */
void xdr_dump(char *xdr, char *buff, int buff_size);

/* judge the XDR packet is valid */
int  xdr_pkt_is_valid(char *xdr, uint32_t bytes);

int  xdr_is_valid_bytes(byte *buff, int size);

void xdr_copy_bytes(byte *dst, byte *src, int size);

/* The interface to manipulate XDR header */
int  get_xdr_pkt_len(struct xdr_hdr_t *hdr);
void get_xdr_city(struct xdr_hdr_t *hdr, char *str, int size);
int  get_xdr_interface(struct xdr_hdr_t *hdr);
void get_xdr_id(struct xdr_hdr_t *hdr, char *str, int size);
int  get_xdr_rat(struct xdr_hdr_t *hdr);
void get_xdr_imsi(struct xdr_hdr_t *hdr, char *str, int size);
void get_xdr_imei(struct xdr_hdr_t *hdr, char *str, int size);
void get_xdr_msisdn(struct xdr_hdr_t *hdr, char *str, int size);

/* The interface to manipulate fileds in XDR packet */
int  get_xdr_u_key(byte *key, char *xdr);
int  get_xdr_s_key(byte *key, char *xdr);
void get_xdr_src_enb_key(char *dst, char *xdr);
void get_xdr_dst_enb_key(char *dst, char *xdr);

byte get_xdr_procedure_type(char *xdr);
byte get_xdr_procedure_status(char *xdr);
byte *get_xdr_procedure_start(char *xdr);
byte *get_xdr_procedure_end(char *xdr);
byte get_xdr_cause(char *xdr);

#endif	/* _XDR_PKT_H_ */
