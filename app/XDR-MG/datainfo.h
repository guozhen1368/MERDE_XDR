#ifndef __XDR_MG_DATAINFO_H__
#define __XDR_MG_DATAINFO_H__

#include <stdint.h>

typedef unsigned char byte;
/*--------------------------------------
上报数据格式：
信令合成xdr：xdr_merge_head_t + xdr_merge_sig_head_t + （xdr_merge_singal_t + ···+ xdr_merge_singal_t）
业务合成xdr：xdr_merge_head_t + xdr_merge_busy_head_t + （xdr_merge_busy_t + ···+xdr_merge_busy_t）
----------------------------------------*/
#pragma pack(1)
struct xdr_merge_head_t
{
	uint16_t length;
	uint16_t city;
	uint8_t  rat;
	uint8_t  xdr_type;
	byte     xdr_ID[16];
	uint64_t imsi;
	uint64_t imei;
	byte     msisdn[16];

};
#pragma pack()

#pragma pack(1)
struct xdr_merge_sig_head_t
{
	uint8_t type;
	uint64_t start_time;
	uint64_t end_time;
	uint64_t start_longitude;
	uint64_t start_latitude;
	uint64_t end_longitude;
	uint64_t end_latitude;
	uint8_t procedure_status;
	uint8_t failure_interface;
	uint16_t failure_cause;
	uint8_t keyword;
	uint32_t eNB_ID;
	uint32_t cell_ID;
	uint16_t mme_group_ID;
	uint8_t mme_code;
	uint16_t tac;
	uint32_t user_IPv4;
	byte user_IPv6[16];
	uint32_t new_eNB_ID;
	uint32_t new_cell_ID;
	uint16_t new_mme_group_ID;
	uint8_t new_mme_code;
	uint16_t new_tac; ////23
	//uint8_t eps_bearer_number;
	unsigned char data[0];
	//some bearer
	// .....

	//uint8_t 单接口 number

};
#pragma pack()

#pragma pack(1)
/* singal interface info */
struct xdr_merge_singal_t {
	uint8_t interfac;         /* [XDR header] */
	byte xdr_id[16];        /* [XDR header] */
	uint8_t procedure_type;    /* [XDR payload] */
	uint64_t procedure_start;/* [XDR payload] */
	uint64_t procedure_end;  /* [XDR payload] */
	uint64_t start_loc_lng;
	uint64_t start_loc_lat;
	uint64_t end_loc_lng;
	uint64_t end_loc_lat;
	uint8_t procedure_status;  /* [XDR payload] */
	uint8_t cause;             /* [XDR payload] */
};
#pragma pack()

#pragma pack(1)
struct xdr_merge_busy_head_t
{

};
#pragma pack()

#pragma pack(1)
struct xdr_merge_busy_t
{

};
#pragma pack()


////////////////////////////////////////////////////////
/*
xdr回填模块上报数据格式：
xdr_merge_user_t + xdr_merge_singal_t + *_priv_t
*/

#pragma pack(1)
/* User info. All the following field get from XDR header */
struct merge_user_t {
	uint16_t lenth;
	uint16_t city;
	uint8_t rat;
	uint64_t imsi;
	uint64_t imei;
	byte msisdn[16];

	struct xdr_merge_singal_t sig;
};
#pragma pack()

#pragma pack(1)
struct s1_mme_priv_t{
	uint8_t keyword;
	uint16_t mme_group_ID;
	uint8_t mme_code;
	uint32_t user_IPv4;
	byte user_IPv6[16];
	uint16_t tac;
	uint16_t new_tac;

	unsigned char bear[0];
	//后续字段成组出现
	/*EPS Bearer Number
	Bearer 1 ID
	Bearer 1 Type
	Bearer 1 QCI
	Bearer 1 Status
	Bearer 1 eNB GTP-TEID
	Bearer 1 SGW GTP-TEID*/

};
#pragma pack()

#pragma pack(1)
struct uu_priv_t{
	uint16_t mme_group_ID;
	uint8_t mme_code;
	uint32_t eNB_ID;
	uint32_t cell_ID;
};
#pragma pack()

#pragma pack(1)
struct x2_priv_t{
	uint16_t mme_group_ID;
	uint8_t mme_code;
	uint32_t eNB_ID;
	uint32_t cell_ID;
	uint32_t new_eNB_ID;
	uint32_t new_cell_ID;
};
#pragma pack()



#endif //__XDR_MG_DATAINFO_H__