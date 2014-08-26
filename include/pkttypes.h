/*
 * (C) Copyright 2010
 * Hu Chunlin <chunlin.hu@gmail.com>
 *
 * pkttypes.h - Define packet type, subtype and protocol.
 *
 */

#ifndef _HEAD_PKTTYPES_5B14EEFD_43D40048_2137398D_H
#define _HEAD_PKTTYPES_5B14EEFD_43D40048_2137398D_H

#if !defined(_MY_DRIVER_)
#ifndef WIN32
#include <inttypes.h>
#else
#include "uinttypes.h"
#endif
#endif

#define PKTTYPE_DATA                 0x00
#define PKTTYPE_DATA_MTP                      0x00
#define PKTTYPE_DATA_LAPD                     0x01
#define PKTTYPE_DATA_PPP                      0x02
#define PKTTYPE_DATA_ATM                      0x03
#define PKTTYPE_DATA_ETHERNET                 0x04
#define PKTTYPE_DATA_FRAMERELAY               0x05
#define PKTTYPE_DATA_TC                       0x06
#define PKTTYPE_DATA_LINUX_SLL                0x07
#define PKTTYPE_DATA_RAW_IP                   0x08
#define PKTTYPE_DATA_IF1                      0x09
#define PKTTYPE_DATA_GTEL_UDP                 0x0A

#define PKTTYPE_HEARTBEAT            0x01
#define PKTTYPE_HEARTBEAT_SOCKET              0x01

#define PKTTYPE_XDR                  0x09

#define PKTTYPE_CDR                  0x10
#define PKTTYPE_CDR_MAP                       0x11
#define PKTTYPE_CDR_ABIS                      0x12
#define PKTTYPE_CDR_BSSAP                     0x13
#define PKTTYPE_CDR_GB                        0x14
#define PKTTYPE_CDR_CAP                       0x15
#define PKTTYPE_CDR_IUCS                      0x16
#define PKTTYPE_CDR_IUPS                      0x17
#define PKTTYPE_CDR_BICC                      0x18
#define PKTTYPE_CDR_H248                      0x19
#define PKTTYPE_CDR_GN                        0x1A
#define PKTTYPE_CDR_GI                        0x1B
#define PKTTYPE_CDR_ICMP                      0x1C
#define PKTTYPE_CDR_FTP                       0x1D
#define PKTTYPE_CDR_RADIUS                    0x1E
#define PKTTYPE_CDR_A11                       0x1F
#define PKTTYPE_CDR_GBCTRL                    0x21
#define PKTTYPE_CDR_IUPSCTRL                  0x22
#define PKTTYPE_CDR_RRC                       0x30
#define PKTTYPE_CDR_X2AP                      0x31
#define PKTTYPE_CDR_SGS                       0x32
#define PKTTYPE_CDR_S1AP                      0x33
#define PKTTYPE_CDR_DIAMETER                  0x34
#define PKTTYPE_CDR_CSFB                      0x3A
#define PKTTYPE_CDR_TCP                       0x40
#define PKTTYPE_CDR_UDP                       0x41
#define PKTTYPE_CDR_HTTP                      0x42
#define PKTTYPE_CDR_WSP                       0x43
#define PKTTYPE_CDR_WAP                       0x44
#define PKTTYPE_CDR_MMS                       0x45
#define PKTTYPE_CDR_MM3                       0x46
#define PKTTYPE_CDR_MM4                       0x47
#define PKTTYPE_CDR_MM7                       0x48
#define PKTTYPE_CDR_MMSMGR                    0x49
#define PKTTYPE_CDR_DNS                       0x4A
#define PKTTYPE_CDR_QQ                        0x50
#define PKTTYPE_CDR_SMTP                      0x51
#define PKTTYPE_CDR_POP3                      0x52
#define PKTTYPE_CDR_IMAP                      0x53
#define PKTTYPE_CDR_RTSP                      0x54
#define PKTTYPE_CDR_SIP                       0x55
#define PKTTYPE_CDR_PARLAY                    0x56
#define PKTTYPE_CDR_PIM                       0x57
#define PKTTYPE_CDR_SMPP                      0x58
#define PKTTYPE_CDR_SMGP                      0x59

#define PKTTYPE_CALLTRACE            0x11

#define PKTTYPE_RAWQUERY             0x12

#define PKTTYPE_CDRQUERY             0x13

#define PKTTYPE_XDRRAWDATA           0x14

#define PKTTYPE_UNKNOWNMSG           0x15

#define PKTTYPE_EVENT                0x18
#define PKTTYPE_EVENT_MAP                     0x11
#define PKTTYPE_EVENT_ABIS                    0x12
#define PKTTYPE_EVENT_ABIS_MR                           0x01
#define PKTTYPE_EVENT_ABIS_HO_SUCCESS                   0x02
#define PKTTYPE_EVENT_ABIS_HO_FAILURE                   0x03
#define PKTTYPE_EVENT_ABIS_ACTIVATION_ASSIGNMENT        0x10
#define PKTTYPE_EVENT_ABIS_RESOURCE_INFORMATION         0x11
#define PKTTYPE_EVENT_BSSAP                   0x13
#define PKTTYPE_EVENT_BSSAP_INTRA_BSC_HO                0x01
#define PKTTYPE_EVENT_BSSAP_INTER_BSC_HO                0x02
#define PKTTYPE_EVENT_BSSAP_HO_FAILURE                  0x03
#define PKTTYPE_EVENT_BSSAP_HO_REQUIRED_REJECT          0x04
#define PKTTYPE_EVENT_BSSAP_HI_SUCCESS                  0x05
#define PKTTYPE_EVENT_BSSAP_HI_FAILURE                  0x06
#define PKTTYPE_EVENT_BSSAP_ASSIGNMENT                  0x10
#define PKTTYPE_EVENT_BSSAP_HANDOVER                    0x11
#define PKTTYPE_EVENT_BSSAP_TERRESTRIAL_RESOURCE        0x12
#define PKTTYPE_EVENT_BSSAP_GENERAL                     0x13
#define PKTTYPE_EVENT_GB                      0x14
#define PKTTYPE_EVENT_GB_CELL_RESELECT                  0x01
#define PKTTYPE_EVENT_GB_CRUX                           0x02
#define PKTTYPE_EVENT_CAP                     0x15
#define PKTTYPE_EVENT_IUCS                    0x16
#define PKTTYPE_EVENT_IUCS_INTRA_BSC_HO                 0x01
#define PKTTYPE_EVENT_IUCS_INTER_BSC_HO                 0x02
#define PKTTYPE_EVENT_IUCS_HO_FAILURE                   0x03
#define PKTTYPE_EVENT_IUCS_HO_REQUIRED_REJECT           0x04
#define PKTTYPE_EVENT_IUCS_HI_SUCCESS                   0x05
#define PKTTYPE_EVENT_IUCS_HI_FAILURE                   0x06
#define PKTTYPE_EVENT_IUPS                    0x17
#define PKTTYPE_EVENT_BICC                    0x18
#define PKTTYPE_EVENT_BICC_IAM                          0x00
#define PKTTYPE_EVENT_BICC_ACM                          0x01
#define PKTTYPE_EVENT_BICC_ANM                          0x02
#define PKTTYPE_EVENT_BICC_REL                          0x03
#define PKTTYPE_EVENT_BICC_RLC                          0x04
#define PKTTYPE_EVENT_H248                    0x19
#define PKTTYPE_EVENT_GN                      0x1A
#define PKTTYPE_EVENT_GN_CP_CREATE                      0x01
#define PKTTYPE_EVENT_GN_CP_UPDATE                      0x02
#define PKTTYPE_EVENT_GN_CP_DELETE                      0x03
#define PKTTYPE_EVENT_GN_SESSION_TIMER                  0x04
#define PKTTYPE_EVENT_GN_SESSION_TERMINATED             0x05
#define PKTTYPE_EVENT_GI                      0x1B
#define PKTTYPE_EVENT_ICMP                    0x1C
#define PKTTYPE_EVENT_FTP                     0x1D
#define PKTTYPE_EVENT_FTP_DATA_TERMINATED               0x01
#define PKTTYPE_EVENT_RADIUS                  0x1E
#define PKTTYPE_EVENT_RADIUS_START                      0x01
#define PKTTYPE_EVENT_RADIUS_STOP                       0x02
#define PKTTYPE_EVENT_RADIUS_ALIVE                      0x03
#define PKTTYPE_EVENT_A11                     0x1F
#define PKTTYPE_EVENT_A11_CREATE                        0x01
#define PKTTYPE_EVENT_A11_UPDATE                        0x02
#define PKTTYPE_EVENT_A11_RELEASE                       0x03
#define PKTTYPE_EVENT_RRC                     0x30
#define PKTTYPE_EVENT_RRC_CELL_MR                       0x01
#define PKTTYPE_EVENT_RRC_UE_MR                         0x02
#define PKTTYPE_EVENT_TCP                     0x40
#define PKTTYPE_EVENT_TCP_CELL_CHANGED                  0x01
#define PKTTYPE_EVENT_TCP_TIMER                         0x02
#define PKTTYPE_EVENT_TCP_TERMINATED                    0x03
#define PKTTYPE_EVENT_UDP                     0x41
#define PKTTYPE_EVENT_UDP_CELL_CHANGED                  0x01
#define PKTTYPE_EVENT_UDP_TIMER                         0x02
#define PKTTYPE_EVENT_UDP_TERMINATED                    0x03
#define PKTTYPE_EVENT_HTTP                    0x42
#define PKTTYPE_EVENT_HTTP_CELL_CHANGED                 0x01
#define PKTTYPE_EVENT_HTTP_TIMER                        0x02
#define PKTTYPE_EVENT_HTTP_TERMINATED                   0x03
#define PKTTYPE_EVENT_WSP                     0x43
#define PKTTYPE_EVENT_WSP_CELL_CHANGED                  0x01
#define PKTTYPE_EVENT_WSP_TIMER                         0x02
#define PKTTYPE_EVENT_WSP_TERMINATED                    0x03
#define PKTTYPE_EVENT_WAP                     0x44
#define PKTTYPE_EVENT_WAP_CELL_CHANGED                  0x01
#define PKTTYPE_EVENT_WAP_TIMER                         0x02
#define PKTTYPE_EVENT_WAP_TERMINATED                    0x03
#define PKTTYPE_EVENT_MMS                     0x45
#define PKTTYPE_EVENT_MMS_TERMINATED                    0x01
#define PKTTYPE_EVENT_MM3                     0x46
#define PKTTYPE_EVENT_MM4                     0x47
#define PKTTYPE_EVENT_MM7                     0x48
#define PKTTYPE_EVENT_MMSMGR                  0x49
#define PKTTYPE_EVENT_DNS                     0x4A
#define PKTTYPE_EVENT_DNS_TERMINATED                    0x01
#define PKTTYPE_EVENT_QQ                      0x50
#define PKTTYPE_EVENT_QQ_CELL_CHANGED                   0x01
#define PKTTYPE_EVENT_QQ_TIMER                          0x02
#define PKTTYPE_EVENT_QQ_TERMINATED                     0x03
#define PKTTYPE_EVENT_SMTP                    0x51
#define PKTTYPE_EVENT_POP3                    0x52
#define PKTTYPE_EVENT_IMAP                    0x53
#define PKTTYPE_EVENT_RTSP                    0x54
#define PKTTYPE_EVENT_SIP                     0x55
#define PKTTYPE_EVENT_PARLAY                  0x56
#define PKTTYPE_EVENT_PIM                     0x57
#define PKTTYPE_EVENT_SMPP                    0x58
#define PKTTYPE_EVENT_SMGP                    0x59

#define CMD_GET_HW_MODEL                      0x2001
#define CMD_GET_HW_VERSION                    0x2002
#define CMD_GET_SW_VERSION                    0x2003
#define CMD_GET_HW_SERIAL_NUM                 0x2004
#define CMD_GET_HW_DESCRIPTION                0x2005
#define CMD_SET_HW_DESCRIPTION                0x2006
#define CMD_GET_ETHER_INFO                    0x2011
#define CMD_SET_ETHER_INFO                    0x2012
#define CMD_GET_MAC                           0x2013
#define CMD_SET_MAC                           0x2014
#define CMD_GET_SERVER_IP                     0x2015
#define CMD_SET_SERVER_IP                     0x2016
#define CMD_GET_GATEWAY_IP                    0x2017
#define CMD_SET_GATEWAY_IP                    0x2018
#define CMD_GET_SYS_TIME                      0x2021
#define CMD_SET_SYS_TIME                      0x2022
#define CMD_GET_NTP_MODE                      0x2023
#define CMD_SET_NTP_MODE                      0x2024
#define CMD_GET_CLK_TYPE                      0x2031
#define CMD_SET_CLK_TYPE                      0x2032
#define CMD_GET_PORTS                         0x2041
#define CMD_GET_GLOBAL_PORT_CFG               0x2042
#define CMD_SET_GLOBAL_PORT_CFG               0x2043
#define CMD_GET_PORT_CFG                      0x2044
#define CMD_SET_PORT_CFG                      0x2045
#define CMD_DEL_PORT_CFG                      0x2046
#define CMD_DOWNLOAD_PORT_CFGFILE             0x2047
#define CMD_UPLOAD_PORT_CFGFILE               0x2048
#define CMD_DOWNLOAD_CH_CFGFILE               0x2051
#define CMD_UPLOAD_CH_CFGFILE                 0x2052
#define CMD_GET_DEV_ID                        0x2061
#define CMD_SET_DEV_ID                        0x2062
#define CMD_GET_PACK_TYPE                     0x2063
#define CMD_SET_PACK_TYPE                     0x2064
#define CMD_GET_DEV_MODE                      0x2065
#define CMD_SET_DEV_MODE                      0x2066
#define CMD_GET_PORT_SDH_MATRIX_CFG           0x2071
#define CMD_SET_PORT_SDH_MATRIX_CFG           0x2072
#define CMD_GET_PORT_MATRIX_CFG               0x2073
#define CMD_SET_PORT_MATRIX_CFG               0x2074
#define CMD_DOWNLOAD_MATRIX_CFGFILE           0x2075
#define CMD_UPLOAD_MATRIX_CFGFILE             0x2076
#define CMD_GAIN_SCAN                         0x2101
#define CMD_CHAN_SCAN                         0x2102
#define CMD_GET_GLOBAL_BERT_CFG               0x2111
#define CMD_SET_GLOBAL_BERT_CFG               0x2112
#define CMD_GET_BERT_CFG                      0x2113
#define CMD_SET_BERT_CFG                      0x2114
#define CMD_DEL_BERT_CFG                      0x2115
#define CMD_START_PRBS                        0x2116
#define CMD_STOP_PRBS                         0x2117
#define CMD_INSERT_ERROR                      0x2118
#define CMD_SYS_REBOOT                        0x2121
#define CMD_SYS_FIRMWARE_UPGRADE              0x2122
#define CMD_SYS_START                         0x2123
#define CMD_SYS_STOP                          0x2124
#define CMD_GET_SYS_RES_REP                   0x2131
#define CMD_GET_LINE_STAT_INFO                0x2141
#define CMD_CLR_LINE_STAT_INFO                0x2142
#define CMD_GET_PORT_STAT_INFO                0x2151
#define CMD_CLR_PORT_STAT_INFO                0x2152
#define CMD_START_PFM_TEST                    0x2153
#define CMD_GET_PORT_PFM_RESULT               0x2154
#define CMD_STOP_PFM_TEST                     0x2155
#define CMD_GET_CH_CFG                        0x2161
#define CMD_GET_CH_STAT_INFO                  0x2162
#define CMD_CLR_CH_STAT_INFO                  0x2163
#define CMD_GET_RAW_DATA                      0x2300
#define CMD_GET_RT_VOICE                      0x2301
#define CMD_GET_CALL_TRACE                    0x2302
#define CMD_SWDBUS_REGISTER                   0x2310
#define CMD_SWDBUS_UNREGISTER                 0x2311
#define CMD_SWDBUS_ADD_PRODUCER               0x2312
#define CMD_SWDBUS_DEL_PRODUCER               0x2313
#define CMD_SWDBUS_UPDATE_FILTER              0x2314
#define CMD_SWDBUS_KEEPALIVE                  0x2315
#define CMD_CT_START                          0x2320
#define CMD_CT_STOP                           0x2321
#define CMD_CT_CDRDATA                        0x2322
#define CMD_CQ_CDR_START                      0x2323
#define CMD_CQ_CDR_STOP                       0x2324
#define CMD_CQ_CDRDATA                        0x2325
#define CMD_SERVER_RAW_QUERY                  0x2326
#define CMD_SERVER_RAWDATA                    0x2327
#define CMD_CT_KEEPALIVE                      0x2328
#define CMD_CQ_KEEPALIVE                      0x2329
#define CMD_WWW_KEEPALIVE                     0x2330
#define CMD_WWW_CT_START                      0x2331
#define CMD_WWW_CT_STOP                       0x2332
#define CMD_WWW_CT_CDRDATA                    0x2333
#define CMD_WWW_CT_RAW_QUERY                  0x2334
#define CMD_WWW_CT_RAWDATA                    0x2335
#define CMD_WWW_CQ_START                      0x2336
#define CMD_WWW_CQ_STOP                       0x2337
#define CMD_WWW_CQ_CDRDATA                    0x2338
#define CMD_WWW_CQ_RAW_QUERY                  0x2339
#define CMD_WWW_CQ_RAWDATA                    0x2340
#define CMD_WWW_CQ_NO_RAWDATA                 0x2341

#define CMD_RNVR_DATA_VIDEO                   0x2201
#define CMD_RNVR_DATA_AUDIO                   0x2202
#define CMD_RNVR_DATA_ALARM                   0x2203
#define CMD_RNVR_DATA_PHOTO                   0x2204
#define CMD_RNVR_DATA_CONNECT                 0x2205
#define CMD_RNVR_DATA_DISCONNECT              0x2206
#define CMD_RNVR_DATA_IPCCONNECT              0x2207
#define CMD_RNVR_DATA_IPCDISCONNECT           0x2208
#define CMD_RNVR_DATA_PERIODPHOTO             0x2209
#define CMD_RNVR_DATA_EVENT                   0x220A
#define CMD_RNVR_DATA_ALARM_IN                0x2210
#define CMD_RNVR_DATA_ALARM_IN_CLEAR          0x2220
#define CMD_RNVR_KEEPALIVE                    0x2211
#define CMD_RNVR_UNKNOWN_HANDLE               0x221F
#define CMD_RNVR_CMD_REALPLAY_START           0x2221
#define CMD_RNVR_CMD_REALPLAY_STOP            0x2222
#define CMD_RNVR_CMD_AL_REPLAY_START          0x2223
#define CMD_RNVR_CMD_AL_REPLAY_STOP           0x2224
#define CMD_RNVR_CMD_GETALARMJPG              0x2225
#define CMD_RNVR_CMD_PTZCONTROL               0x2226
#define CMD_RNVR_CMD_GET_CFG                  0x2227
#define CMD_RNVR_CMD_SET_CFG                  0x2228
#define CMD_RNVR_CMD_GET_DEVCFG               0x2229
#define CMD_RNVR_CMD_SET_DEVCFG               0x222A
#define CMD_RNVR_CMD_GET_PERIODPHOTO          0x222B
#define CMD_RNVR_CMD_SET_SWITCH_OUT           0x222C
#define CMD_RNVR_CMD_GET_SWITCH_OUT           0x222D

#define PKTTYPE_KPCDR                  0x30
#define PKTTYPE_KPCDR_MAP                     0x11
#define PKTTYPE_KPCDR_ABIS                    0x12
#define PKTTYPE_KPCDR_BSSAP                   0x13
#define PKTTYPE_KPCDR_GB                      0x14
#define PKTTYPE_KPCDR_CAP                     0x15
#define PKTTYPE_KPCDR_IUCS                    0x16
#define PKTTYPE_KPCDR_IUPS                    0x17
#define PKTTYPE_KPCDR_BICC                    0x18
#define PKTTYPE_KPCDR_H248                    0x19
#define PKTTYPE_KPCDR_GN                      0x1A
#define PKTTYPE_KPCDR_GI                      0x1B
#define PKTTYPE_KPCDR_ICMP                    0x1C
#define PKTTYPE_KPCDR_FTP                     0x1D
#define PKTTYPE_KPCDR_RADIUS                  0x1E
#define PKTTYPE_KPCDR_BSSAPVOC                0x1F
#define PKTTYPE_KPCDR_TCP                     0x40
#define PKTTYPE_KPCDR_UDP                     0x41
#define PKTTYPE_KPCDR_HTTP                    0x42
#define PKTTYPE_KPCDR_WSP                     0x43
#define PKTTYPE_KPCDR_WAP                     0x44
#define PKTTYPE_KPCDR_MMS                     0x45
#define PKTTYPE_KPCDR_MM3                     0x46
#define PKTTYPE_KPCDR_MM4                     0x47
#define PKTTYPE_KPCDR_MM7                     0x48
#define PKTTYPE_KPCDR_MMSMGR                  0x49
#define PKTTYPE_KPCDR_DNS                     0x4A
#define PKTTYPE_KPCDR_QQ                      0x50
#define PKTTYPE_KPCDR_SMTP                    0x51
#define PKTTYPE_KPCDR_POP3                    0x52
#define PKTTYPE_KPCDR_IMAP                    0x53
#define PKTTYPE_KPCDR_RTSP                    0x54
#define PKTTYPE_KPCDR_SIP                     0x55
#define PKTTYPE_KPCDR_PARLAY                  0x56
#define PKTTYPE_KPCDR_PIM                     0x57
#define PKTTYPE_KPCDR_SMPP                    0x58
#define PKTTYPE_KPCDR_SMGP                    0x59


#define PKTTYPE_MSG_INFORMATION        0x31

#define PKTTYPE_STATISTICS             0x32
#define PKTTYPE_STATISTICS_IP_FLOW            0x01
#define PKTTYPE_STATISTICS_SCTP_CHUNK         0x02
#define PKTTYPE_STATISTICS_SCTP_DATA_FLOW     0x03
#define PKTTYPE_STATISTICS_MTP_FLOW           0x04
#define PKTTYPE_STATISTICS_FLOW               0x05

#define PKTTYPE_DPI                    0x33

#define PKTTYPE_REPORT                 0x40
#define PKTTYPE_REPORT_GATEWAY                0x01
#define PKTTYPE_REPORT_STATUS                 0x02
#define PKTTYPE_REPORT_ALARM                  0x03

#define PKTTYPE_SERVER_NODBUS        0x50
#define PKTTYPE_SERVER_TMSI2IMSI     0x51
#define PKTTYPE_SERVER_TMSI2IMEI     0x52
#define PKTTYPE_SERVER_IMSI2MSISDN   0x53
#define PKTTYPE_SERVER_TMSI2CDRID    0x54
#define PKTTYPE_SERVER_TMSI2LOCATION 0x55
#define PKTTYPE_SERVER_TMSI2SESSIONID 0x56
#define PKTTYPE_SERVER_MSISDN2IMSI   0x57
#define PKTTYPE_SERVER_IMSI2IMEI     0x58

#define PKTTYPE_KEEPALIVE            0xFE

#define PKTTYPE_WILDCARD             0xFF

/* Command related definitions. */
#define PKTTYPE_CMD     0x20

#define PKTTYPE_CMD_MIN 0x20
#define PKTTYPE_CMD_MAX 0x2b

#define PKTTYPE_CMDACK  0x08
#define PKTTYPE_CMDNACK 0x04

/* Protocol */
#define PKTPROTOCOL_AUTO            0x00
#define PKTPROTOCOL_ABIS            0x01
#define PKTPROTOCOL_MOBIS           0x02
#define PKTPROTOCOL_SS7             0x03
#define PKTPROTOCOL_GB              0x04


#if defined(__cplusplus)
extern "C" {
#endif

int pkttypes_typestr2type(const char *typestr, uint8_t *type);
const char *pkttypes_type2typestr(uint8_t type);
int pkttypes_subtypestr2subtype(uint8_t type, const char *subtypestr, uint8_t *subtype);
const char *pkttypes_subtype2subtypestr(uint8_t type, uint8_t subtype);
int pkttypes_protocolstr2protocol(uint8_t type, uint8_t subtype, const char *protocolstr, uint8_t *protocol);
const char *pkttypes_protocol2protocolstr(uint8_t type, uint8_t subtype, uint8_t protocol);

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef _HEAD_PKTTYPES_5B14EEFD_43D40048_2137398D_H */

