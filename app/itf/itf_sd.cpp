/*
 * (C) Copyright 2008
 * Beijing HLYT Technology Co., Ltd.
 *
 * itf_sd.h - A brief description to describe this file.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <map>
#include "os.h"
#include "apfrm.h"
#include "aplog.h"
#include "cconfig.h"
#include "misc.h"
#include "pkttypes.h"
#include "pkt.h"
#include "adapter.h"
#include "swdbus.h"
#include "adapter_ss.h"
#include "adapter_archive.h"
#include "timermgr.h"
#include "Pkt.h"

#define SD_FPRINTF_JUDGE(nSize, fp, fmt, field) \
	if ((field)) \
		nSize += fprintf((FILE *)(fp), (fmt), (field)); \
	else \
		nSize += fprintf((FILE *)(fp), ";");

static void *g_timer          = NULL;

static void *g_dbus_cdr       = NULL;
static void *g_dbus_event     = NULL;

static void *sms_adap_to      = NULL;
static void *call_adap_to     = NULL;
static void *power_adap_to    = NULL;
static void *switch_adap_to   = NULL;
static void *location_adap_to = NULL;

std::map<unsigned int, unsigned int> ci2lac;

static char *cfgfile          = NULL;

static uint64_t g_recieve_msg     = 0ull;

static uint64_t g_call_msg        = 0ull;
static uint64_t g_sms_msg         = 0ull;
static uint64_t g_location_msg    = 0ull;
static uint64_t g_switch_msg      = 0ull;
static uint64_t g_power_msg       = 0ull;

struct pktrec {
	pkt_hdr hdr;
	Pkt    *rec;
};

static void TimerCallback(unsigned int s, unsigned int ns,void *data, void *arg)
{
	LOG("Got %llu messages [call %llu, sms %llu, "
			"location %llu, switch %llu, power %llu]", g_recieve_msg,
			g_call_msg, g_sms_msg, g_location_msg, g_switch_msg, g_power_msg);
}

static void itf_sd_show_usage(char *progname)
{
	printf("        --cfgfile <filename>: configuration file name\n");
}

static int itf_sd_parse_args(int argc, char **argv)
{
	int i = 0;

	while (i < argc) {
		if (strcmp(argv[i], "--cfgfile") == 0) {
			if (((i + 1) >= argc) || (argv[i + 1][0] == '-'))
				return -1;

			i++;
			cfgfile = argv[i];
		}
		else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			return -1;
		}
		i++;
	}

	if (cfgfile == NULL) {
		fprintf(stderr, "No configuration file specified.\n");
		return -1;
	}

	return 0;
}

static const char *GetTimeString(time_t s)
{
	static char buf[32] = { 0 };
	struct tm tm;

	localtime_r(&s, &tm);
	sprintf(buf, "%04d%02d%02d%02d%02d%02d",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
	LOGDEBUG("Timestamp: %u, %s.", s, buf);

	return buf;
}

static void ChangeComma(unsigned char *s, uint32_t newlen)
{
	uint32_t i = 0;

	for (i = 0; i < newlen; ++i, ++s) {
		if ((*s == 0x0d) || (*s == 0x0a) || (*s == 0x3b) || (*s == 0x03))
			*s = '.';
	}
}

static char *GetSMSContent(unsigned char *s, uint32_t len, uint8_t scheme,
		uint32_t &newlen)
{
	static unsigned char buf[384];
	unsigned char *p;

	if ((s == NULL) || (len == 0))
		return NULL;

	newlen = 0;

	LGWRINFO(s, len, "Original SMS content (scheme %u, len %u):", scheme, len);
	if (scheme == 0) {
		newlen = decode_bit7(s, len, buf);
		p = buf;
	}
	else if (scheme == 8) {
		int rc;

		rc = decode_ucs2(s, len, buf, 382);
		if (rc == -1) {
			LGWRERROR(s, len, "SMS (scheme %u, len %u): Failed to convert",
					scheme, len);
			return NULL;
		}
		newlen = (uint32_t)rc;
		buf[newlen] = 0;
		p = buf;
	}
	else {
		newlen = len;
		p = s;
	}
	ChangeComma(p, newlen);
	LGWRINFO(p, newlen, "Converted SMS content (len %u): %s",
			newlen, (char *)p);

	return (char *)p;
}

static char *GetToString(char *to)
{
	char *p = to;

	if (p) {
		int len = strlen(p);

		if ((len > 5) &&
				(p[0] == '1') && (p[1] == '7') && (p[2] == '9') && (p[3] == '5') && (p[4] == '1')) {
			len -= 5;
			p += 5;
		}

		if ((len > 2) && (p[0] == '0') && (p[1] != '0'))
			p += 1;
		else if ((len == 13) &&
				((p[0] == '8') && (p[1] == '6') && (p[2] == '1'))) {
			p += 2;
		}
		LOGDEBUG("Peer Number: %s -> %s", to, p);
	}

	return p;
}

static char *GetMSISDN(Pkt* pkt)
{
	static char buf[32] = { 0 };
	char *p = buf;

	if (pkt->tdata.cdrbssap.u64MSISDN == 0)
		return NULL;

	sprintf(buf, "%llu", (unsigned long long)pkt->tdata.cdrbssap.u64MSISDN);
	if ((strlen(buf) == 13) &&
			((buf[0] == '8') && (buf[1] == '6') && (buf[2] == '1')))
		p += 2;
	LOGDEBUG("MSISDN: %s -> %s", buf, p);

	return p;
}

off_t SMSOutputCallback(void *fp, void *buf, int len)
{
	Pkt *pkt = ((struct pktrec *)buf)->rec;
	int nSize = 0;
	char *p = NULL;
	uint32_t newlen = 0;

	if ((pkt->tdata.cdrbssap.u8SMType != 1 /* Submit */) &&
			(pkt->tdata.cdrbssap.u8SMType != 3 /* Deliver */))
		return 0;

	if ((pkt->tdata.cdrbssap.u8SMDataCodingScheme & 0x0c) == 0x0c) {
		LGWRERROR((unsigned char *)(pkt->tdata.cdrbssap.u8SMSUserData),
				pkt->tdata.cdrbssap.u16SMLength,
				"SMS: invalid scheme 0x%02x",
				pkt->tdata.cdrbssap.u8SMDataCodingScheme);
		return 0;
	}

	nSize += fprintf((FILE *)fp, "6;"); //CDR类型

	nSize += fprintf((FILE *)fp, "%s;",
			GetTimeString((time_t)(pkt->u32ReportTimeS)));  //reporttime

	p = GetMSISDN(pkt);
	SD_FPRINTF_JUDGE(nSize, fp, "%s;", p); //MSISDN, 去掉前缀86

	nSize += fprintf((FILE *)fp, "%u;",
			(pkt->tdata.cdrbssap.u8CDRType == 12) ? 0 : 1); // 呼叫类型

	SD_FPRINTF_JUDGE(nSize, fp, "%llu;",
			(unsigned long long)(pkt->tdata.cdrbssap.u64IMSI)); //IMSI

	SD_FPRINTF_JUDGE(nSize, fp, "%15llu;",
			(unsigned long long)(pkt->tdata.cdrbssap.u64IMEI)); //IMEI

	p = GetToString(pkt->tdata.cdrbssap.cPeerNumber);
	SD_FPRINTF_JUDGE(nSize, fp, "%s;", p); //对方号码

	nSize += fprintf((FILE *)fp, "0;"); //通话时长， call类型专有
	nSize += fprintf((FILE *)fp, ";"); //漫游类型， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方通话地区号， 甲方
	SD_FPRINTF_JUDGE(nSize, fp, "%u;", pkt->tdata.cdrbssap.u32DPC); //MSC_ID
	SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LAC);//LAC_ID
	if (pkt->u8Subtype == PKTTYPE_CDR_BSSAP) {
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;",
				pkt->tdata.cdrbssap.u16CI ? pkt->tdata.cdrbssap.u16CI : pkt->tdata.cdrbssap.u16SAC);//CELL_ID
	}
	else {
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16SAC);//CELL_ID
	}
	nSize += fprintf((FILE *)fp, ";");// 对方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";");// 境外类型， 甲方
	nSize += fprintf((FILE *)fp, ";");// 特服标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 废单标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 运营商， 甲方
	nSize += fprintf((FILE *)fp, ";");// 文件名， 甲方
	nSize += fprintf((FILE *)fp, ";");// 预留字段1
	nSize += fprintf((FILE *)fp, ";");// 预留字段2
	nSize += fprintf((FILE *)fp, ";");// 预留字段3
	nSize += fprintf((FILE *)fp, ";");// 预留字段4

	p = GetSMSContent((unsigned char *)(pkt->tdata.cdrbssap.u8SMSUserData),
			pkt->tdata.cdrbssap.u16SMLength,
			pkt->tdata.cdrbssap.u8SMDataCodingScheme & 0x0C, newlen);
	if (p && (newlen > 0)) {
		fwrite(p, 1, newlen, (FILE *)fp); //短信内容，编码转换
		nSize += newlen;
	}
	nSize += fprintf((FILE *)fp, ";\r\n");

	return nSize;
}

off_t LocationOutputCallback(void *fp, void *buf, int len)
{
	Pkt *pkt = ((struct pktrec *)buf)->rec;
	int nSize = 0;
	char *p = NULL;

	if (pkt->tdata.cdrbssap.u8MNC == 0x03) { /* CDMA */
		nSize += fprintf((FILE *)fp, "1;"); //CDR类型
	}
	else {
		if (pkt->tdata.cdrbssap.u8CDRType == 6)
			nSize += fprintf((FILE *)fp, "0;"); //CDR类型
		else
			nSize += fprintf((FILE *)fp, "1;"); //CDR类型
	}

	nSize += fprintf((FILE *)fp, "%s;",
			GetTimeString((time_t)(pkt->u32ReportTimeS)));  //reporttime

	p = GetMSISDN(pkt);
	SD_FPRINTF_JUDGE(nSize, fp, "%s;", p); //MSISDN, 去掉前缀86

	nSize += fprintf((FILE *)fp, ";"); // 呼叫类型

	SD_FPRINTF_JUDGE(nSize, fp, "%llu;",
			(unsigned long long)(pkt->tdata.cdrbssap.u64IMSI)); //IMSI

	SD_FPRINTF_JUDGE(nSize, fp, "%15llu;",
			(unsigned long long)(pkt->tdata.cdrbssap.u64IMEI)); //IMEI

	nSize += fprintf((FILE *)fp, ";"); //对方号码
	nSize += fprintf((FILE *)fp, ";"); //通话时长， call类型专有
	nSize += fprintf((FILE *)fp, ";"); //漫游类型， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方通话地区号， 甲方
	SD_FPRINTF_JUDGE(nSize, fp, "%u;", pkt->tdata.cdrbssap.u32DPC); //MSC_ID
	if (pkt->tdata.cdrbssap.u8MNC == 0x03) { /* CDMA */
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LAC);//LAC_ID
		if (pkt->u8Subtype == PKTTYPE_CDR_BSSAP) {
			SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16CI);//CELL_ID
		}
		else {
			SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16SAC);//CELL_ID
		}
	}
	else {
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LastLAC);//LAC_ID
		if (pkt->u8Subtype == PKTTYPE_CDR_BSSAP) {
			SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LastCI);//CELL_ID
		}
		else {
			SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LastSAC);//CELL_ID
		}
	}
	nSize += fprintf((FILE *)fp, ";");// 对方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";");// 境外类型， 甲方
	nSize += fprintf((FILE *)fp, ";");// 特服标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 废单标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 运营商， 甲方
	nSize += fprintf((FILE *)fp, ";");// 文件名， 甲方
	nSize += fprintf((FILE *)fp, ";");// 预留字段1
	nSize += fprintf((FILE *)fp, ";");// 预留字段2
	nSize += fprintf((FILE *)fp, ";");// 预留字段3
	nSize += fprintf((FILE *)fp, ";");// 预留字段4
	nSize += fprintf((FILE *)fp, ";\r\n");// 短信内容，编码转换, 无内容填空

	return nSize;
}

off_t PowerOutputCallback(void *fp, void *buf, int len)
{
	Pkt *pkt = ((struct pktrec *)buf)->rec;
	int nSize = 0;
	char *p = NULL;

	nSize += fprintf((FILE *)fp, "%u;",
			(pkt->tdata.cdrbssap.u8CDRType == 4) ? 3 : 4); //CDR类型

	nSize += fprintf((FILE *)fp, "%s;",
			GetTimeString((time_t)(pkt->u32ReportTimeS)));  //reporttime

	p = GetMSISDN(pkt);
	SD_FPRINTF_JUDGE(nSize, fp, "%s;", p); //MSISDN, 去掉前缀86

	nSize += fprintf((FILE *)fp, ";"); // 呼叫类型

	SD_FPRINTF_JUDGE(nSize, fp, "%llu;",
			(unsigned long long)(pkt->tdata.cdrbssap.u64IMSI)); //IMSI

	SD_FPRINTF_JUDGE(nSize, fp, "%15llu;",
			(unsigned long long)(pkt->tdata.cdrbssap.u64IMEI)); //IMEI

	nSize += fprintf((FILE *)fp, ";"); //对方号码
	nSize += fprintf((FILE *)fp, ";"); //通话时长， call类型专有
	nSize += fprintf((FILE *)fp, ";"); //漫游类型， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方通话地区号， 甲方
	SD_FPRINTF_JUDGE(nSize, fp, "%u;", pkt->tdata.cdrbssap.u32DPC); //MSC_ID
	if ((pkt->tdata.cdrbssap.u8MNC == 0x03) /* CDMA */ ||
			(pkt->tdata.cdrbssap.u8CDRType == 5)) /* IMSI Detach */ {
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LAC);//LAC_ID
		if (pkt->u8Subtype == PKTTYPE_CDR_BSSAP) {
			SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16CI);//CELL_ID
		}
		else {
			SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16SAC);//CELL_ID
		}
	}
	else {
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LastLAC);//LAC_ID
		if (pkt->u8Subtype == PKTTYPE_CDR_BSSAP) {
			SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LastCI);//CELL_ID
		}
		else {
			SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LastSAC);//CELL_ID
		}
	}
	nSize += fprintf((FILE *)fp, ";");// 对方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";");// 境外类型， 甲方
	nSize += fprintf((FILE *)fp, ";");// 特服标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 废单标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 运营商， 甲方
	nSize += fprintf((FILE *)fp, ";");// 文件名， 甲方
	nSize += fprintf((FILE *)fp, ";");// 预留字段1
	nSize += fprintf((FILE *)fp, ";");// 预留字段2
	nSize += fprintf((FILE *)fp, ";");// 预留字段3
	nSize += fprintf((FILE *)fp, ";");// 预留字段4
	nSize += fprintf((FILE *)fp, ";\r\n");// 短信内容， 编码转换

	return nSize;
}

off_t SwitchOutputCallback(void *fp, void *buf, int len)
{
	Pkt *pkt = ((struct pktrec *)buf)->rec;
	int nSize = 0;
	char *p = NULL;

	nSize += fprintf((FILE *)fp, "2;"); //CDR类型

	nSize += fprintf((FILE *)fp, "%s;",
			GetTimeString((time_t)(pkt->u32ReportTimeS)));  //reporttime

	p = GetMSISDN(pkt);
	SD_FPRINTF_JUDGE(nSize, fp, "%s;", p);  //MSISDN, 去掉前缀86

	nSize += fprintf((FILE *)fp, "%u;",
			(pkt->tdata.eventbssap.u8CDRType == 1) ? 0 : 1); // 呼叫类型

	SD_FPRINTF_JUDGE(nSize, fp, "%llu;",
			(unsigned long long)(pkt->tdata.eventbssap.u64IMSI));  //IMSI

	SD_FPRINTF_JUDGE(nSize, fp, "%15llu;",
			(unsigned long long)(pkt->tdata.eventbssap.u64IMEI));  //IMEI

	p = GetToString(pkt->tdata.eventbssap.cPeerNumber);
	SD_FPRINTF_JUDGE(nSize, fp, "%s;", p); //对方号码

	nSize += fprintf((FILE *)fp, "%u;", pkt->tdata.eventbssap.u32Duration / 1000); //通话时长

	nSize += fprintf((FILE *)fp, ";"); //漫游类型， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方通话地区号， 甲方
	SD_FPRINTF_JUDGE(nSize, fp, "%u;", pkt->tdata.eventbssap.u32ToDPC);//MSC_ID
	SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.eventbssap.u16FromLAC);//LAC_ID
	if (pkt->u8Subtype == PKTTYPE_EVENT_BSSAP) {
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.eventbssap.u16ToCI);//CELL_ID
	}
	else {
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.eventbssap.u16ToSAC);//CELL_ID
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;",
				pkt->tdata.eventbssap.u16ToSAC ? pkt->tdata.eventbssap.u16ToSAC : pkt->tdata.eventbssap.u16ToCI);//CELL_ID
	}
	nSize += fprintf((FILE *)fp, ";");// 对方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";");// 境外类型， 甲方
	nSize += fprintf((FILE *)fp, ";");// 特服标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 废单标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 运营商， 甲方
	nSize += fprintf((FILE *)fp, ";");// 文件名， 甲方
	nSize += fprintf((FILE *)fp, ";");// 预留字段1
	nSize += fprintf((FILE *)fp, ";");// 预留字段2
	nSize += fprintf((FILE *)fp, ";");// 预留字段3
	nSize += fprintf((FILE *)fp, ";");// 预留字段4
	nSize += fprintf((FILE *)fp, ";\r\n");// 短信内容

	return nSize;
}

static uint32_t GetCallTime(Pkt *pkt)
{
	uint32_t diff = 0, cs, ds;

	cs = pkt->tdata.cdrbssap.u32ConnectTime;
	if (cs == 0) {
		if (pkt->tdata.cdrbssap.u8MNC != 0x03) {
			/* Not CDMA, we must have CONNECT to calculate call
			 * duration. */
			return 0;
		}
		cs = pkt->tdata.cdrbssap.u32ConnectAckTime;
		if (cs == 0) {
			cs = pkt->tdata.cdrbssap.u32AssignmentTime;
			if (cs == 0) {
				cs = pkt->tdata.cdrbssap.u32AssignmentCompleteTime;
				if (cs == 0)
					return 0;
			}
		}
	}

	ds = pkt->tdata.cdrbssap.u32DisconnectTime;
	if (ds == 0) {
		ds = pkt->tdata.cdrbssap.u32CCReleaseTime;
		if (ds == 0) {
			ds = pkt->tdata.cdrbssap.u32CCReleaseCompleteTime;
			if (ds == 0) {
				ds = pkt->tdata.cdrbssap.u32SCCPReleaseTime;
				if (ds == 0) {
					ds = pkt->tdata.cdrbssap.u32SCCPReleaseCompleteTime;
					if (ds == 0)
						return 0;
				}
			}
		}
	}

	if (ds > cs)
		diff = ds - cs;
	LOGDEBUG("Call starts at %u, ends at %u, duration %u", cs, ds, diff);

	return diff;
}

off_t CallOutputCallback(void *fp, void *buf, int len)
{
	Pkt *pkt = ((struct pktrec *)buf)->rec;
	int nSize = 0;
	char *p = NULL;

	nSize += fprintf((FILE *)fp, "5;"); //CDR类型

	nSize += fprintf((FILE *)fp, "%s;",
			GetTimeString((time_t)(pkt->u32ReportTimeS)));  //reporttime

	p = GetMSISDN(pkt);
	SD_FPRINTF_JUDGE(nSize, fp, "%s;", p); //MSISDN, 去掉前缀86

	nSize += fprintf((FILE *)fp, "%u;",
			(pkt->tdata.cdrbssap.u8CDRType == 1) ? 0 : 1); // 呼叫类型

	SD_FPRINTF_JUDGE(nSize, fp, "%llu;",
			(unsigned long long)(pkt->tdata.cdrbssap.u64IMSI)); //IMSI

	SD_FPRINTF_JUDGE(nSize, fp, "%15llu;",
			(unsigned long long)(pkt->tdata.cdrbssap.u64IMEI)); //IMEI

	p = GetToString(pkt->tdata.cdrbssap.cPeerNumber);
	SD_FPRINTF_JUDGE(nSize, fp, "%s;", p); //对方号码

	nSize += fprintf((FILE *)fp, "%u;", GetCallTime(pkt) / 1000); //通话时长， call类型专有

	nSize += fprintf((FILE *)fp, ";"); //漫游类型， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";"); //本方通话地区号， 甲方
	SD_FPRINTF_JUDGE(nSize, fp, "%u;", pkt->tdata.cdrbssap.u32DPC); //MSC_ID
	SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16LAC);//LAC_ID
	if (pkt->u8Subtype == PKTTYPE_CDR_BSSAP) {
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;",
				pkt->tdata.cdrbssap.u16CI ? pkt->tdata.cdrbssap.u16CI : pkt->tdata.cdrbssap.u16SAC);//CELL_ID
	}
	else {
		SD_FPRINTF_JUDGE(nSize, fp, "%hu;", pkt->tdata.cdrbssap.u16SAC);//CELL_ID
	}
	nSize += fprintf((FILE *)fp, ";");// 对方归属地区号， 甲方
	nSize += fprintf((FILE *)fp, ";");// 境外类型， 甲方
	nSize += fprintf((FILE *)fp, ";");// 特服标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 废单标志， 甲方
	nSize += fprintf((FILE *)fp, ";");// 运营商， 甲方
	nSize += fprintf((FILE *)fp, ";");// 文件名， 甲方
	nSize += fprintf((FILE *)fp, ";");// 预留字段1
	nSize += fprintf((FILE *)fp, ";");// 预留字段2
	nSize += fprintf((FILE *)fp, ";");// 预留字段3
	nSize += fprintf((FILE *)fp, ";");// 预留字段4
	nSize += fprintf((FILE *)fp, ";\r\n");// 短信内容

	return nSize;
}

static bool InitDbus(uint8_t type, uint8_t subtype, void *&dbus)
{
	dbus = swdbus_env_open(ap_is_running);
	if (dbus == NULL) {
		LOGERROR("Failed to create dbus with type 0x%02x/0x%02x", type,subtype);
		return false;
	}

	if (swdbus_register_consumer(dbus, type, subtype) < 0) {
		LOGERROR("Failed to register consumer: 0x%02x/0x%02x", type, subtype);
		if (dbus) {
			swdbus_close(dbus);
			dbus = NULL;
		}
		return false;
	}

	return true;
}

static void itf_sd_init_lacci_mapping(unsigned long hd)
{
	char value[256];
	unsigned int lac, ci;
	int i, cnt, valid = 0;

	cnt = CfgGetCount(hd, "LAC.CI", "map", 1);
	for (i = 1; i <= cnt; ++i) {
		std::map<unsigned int, unsigned int>::iterator it;

		if (CfgGetValue(hd, "LAC.CI", "map", value, i, 1) != 0) {
			LOGERROR("LAC/CI Mapping: Failed to load item %d", i);
			continue;
		}

		if (sscanf(value, "%u %u", &lac, &ci) != 2) {
			LOGERROR("LAC/CI Mapping: Failed to load item %d: %s", i, value);
			continue;
		}

		if ((it = ci2lac.find(ci)) != ci2lac.end()) {
			LOGERROR("LAC/CI Mapping: duplicate item %d: %s, "
					"previously linked to %u", i, value, it->second);
			continue;
		}
		ci2lac[ci] = lac;
		LOGDEBUG("LAC/CI Mapping: item %d: %s, mapping added.", i, value);
		++valid;
	}
	LOG("LAC/CI Mapping: total %d mappings added.", valid);
}

static int itf_sd_init()
{
	unsigned long hd = 0;

	if ((hd = CfgInitialize(cfgfile)) == 0ul) {
		LOGERROR("Parsing configuration file [%s] failed.", cfgfile);
		return -1;
	}

	call_adap_to = adapter_register_archive(hd,
			(char *)"Call.Archive", CallOutputCallback);
	sms_adap_to = adapter_register_archive(hd,
			(char *)"SMS.Archive", SMSOutputCallback);
	power_adap_to = adapter_register_archive(hd,
			(char *)"Power.Archive", PowerOutputCallback);
	switch_adap_to = adapter_register_archive(hd,
			(char *)"Switch.Archive", SwitchOutputCallback);
	location_adap_to = adapter_register_archive(hd,
			(char *)"Location.Archive", LocationOutputCallback);

	if ((call_adap_to == NULL) || (power_adap_to == NULL) ||
			(sms_adap_to == NULL) || (switch_adap_to == NULL) ||
			(location_adap_to == NULL)) {
		CfgInvalidate(hd);
		return -1;
	}

	itf_sd_init_lacci_mapping(hd);

	CfgInvalidate(hd);

	if (!InitDbus(PKTTYPE_CDR, PKTTYPE_CDR_BSSAP, g_dbus_cdr))
		return -1;
	if (!InitDbus(PKTTYPE_EVENT, PKTTYPE_EVENT_BSSAP, g_dbus_event))
		return -1;

	swdbus_update_filter(
			g_dbus_event, filter_parse((char *)"(PROTOCOL IN [1, 2])"));

	g_timer = timer_mgr_create_timerlist(NULL, TIMERMGR_SOURCE_LOCAL,
			TimerCallback, NULL, "SendCDRConvertStatistics");
	timer_mgr_add_timer_periodic(g_timer, 300, NULL);

	return 0;
}

void itf_sd_exit()
{
	if (g_timer) {
		timer_mgr_release_timerlist(NULL, g_timer);
		g_timer = NULL;
	}

	if (g_dbus_event) {
		swdbus_close(g_dbus_event);
		g_dbus_event = NULL;
	}

	if (g_dbus_cdr) {
		swdbus_close(g_dbus_cdr);
		g_dbus_cdr = NULL;
	}

	if (sms_adap_to) {
		adapter_close(sms_adap_to);
		sms_adap_to = NULL;
	}

	if (call_adap_to) {
		adapter_close(call_adap_to);
		call_adap_to = NULL;
	}

	if (location_adap_to) {
		adapter_close(location_adap_to);
		location_adap_to = NULL;
	}

	if (switch_adap_to) {
		adapter_close(switch_adap_to);
		switch_adap_to = NULL;
	}

	if (power_adap_to) {
		adapter_close(power_adap_to);
		power_adap_to = NULL;
	}
}

static pkt_hdr *ReadDbus()
{
	pkt_hdr *ph = NULL;
	static char i = 0;
	int cnt = 0;

	for (cnt = 0; cnt < 2; ++cnt) {
		if (++i % 2)
			ph = (pkt_hdr *)swdbus_read(g_dbus_cdr);
		else
			ph = (pkt_hdr *)swdbus_read(g_dbus_event);

		if (ph)
			return ph;
	}

	return NULL;
}

static int itf_sd_run(long argc, unsigned long data)
{
	pkt_hdr *ph = NULL;
	Pkt rec;
	struct pktrec tmp;
	int i = 0;

	if (itf_sd_init() < 0) {
		itf_sd_exit();
		return -1;
	}

	while (ap_is_running()) {
		if (++i == 5000) {
			timer_mgr_run_timerlist(NULL, g_timer, 0, 0);
			i = 0;
		}
		if ((ph = ReadDbus()) == NULL) {
			SLEEP_US(5000);
			continue;
		}

		if ((pkthdr_get_type(ph) != PKTTYPE_CDR) &&
				(pkthdr_get_type(ph) != PKTTYPE_EVENT)) {
			LGWRERROR(ph, pkthdr_get_plen(ph), "Wrong cdr received, type %u, subtype %u.",
					pkthdr_get_type(ph), pkthdr_get_subtype(ph));
			free(ph);
			continue;
		}
		if ((pkthdr_get_subtype(ph) != PKTTYPE_CDR_BSSAP /* PKTTYPE_EVENT_BSSAP */) &&
				(pkthdr_get_subtype(ph) != PKTTYPE_CDR_IUCS /* PKTTYPE_EVENT_IUCS */)) {
			LGWRERROR(ph, pkthdr_get_plen(ph), "Wrong subtype of cdr/event");
			free(ph);
			continue;
		}
		if (rec.Decode((char *)ph, pkthdr_get_plen(ph)) < 0) {
			LGWRERROR(ph, pkthdr_get_plen(ph), "Failed to decode packet:");
			free(ph);
			continue;
		}

		++g_recieve_msg;
		tmp.rec = (Pkt *)&rec;
		pkthdr_set_sync(&(tmp.hdr));
		pkthdr_set_ts(&(tmp.hdr), rec.u32ReportTimeS, rec.u32ReportTimeNS);

		switch (rec.u8Type)
		{
		case PKTTYPE_CDR:
			if (rec.tdata.cdrbssap.u16LAC == 0) {
				std::map<unsigned int, unsigned int>::iterator it;

				it = ci2lac.find((unsigned int)rec.tdata.cdrbssap.u16CI);
				if (it != ci2lac.end()) {
					rec.tdata.cdrbssap.u16LAC = it->second;
					LOGDEBUG("LAC updated to %u for CI %u",
							it->second, it->first);
				}
				else {
					LOGDEBUG("LAC unknown for CI %u", rec.tdata.cdrbssap.u16CI);
				}
			}
			switch (rec.tdata.cdrbssap.u8CDRType) {
				case 1:
				case 2:
					/* 通话 */
					adapter_write(call_adap_to, &tmp, sizeof(rec));
					++g_call_msg;
					break;
				case 4:
				case 5:
					/* 开关机 */
					++g_power_msg;
					adapter_write(power_adap_to, &tmp, sizeof(rec));
					break;
				case 6:
				case 7:
					/* 正常/周期位置更新 */
					++g_location_msg;
					adapter_write(location_adap_to, &tmp, sizeof(rec));
					break;
				case 12:
				case 13:
					/* SMS */
					++g_sms_msg;
					adapter_write(sms_adap_to, &tmp, sizeof(rec));
					break;
			}
			break;

		case PKTTYPE_EVENT:
			if (rec.tdata.eventbssap.u16FromLAC == 0) {
				std::map<unsigned int, unsigned int>::iterator it;

				it = ci2lac.find((unsigned int)rec.tdata.eventbssap.u16ToCI);
				if (it != ci2lac.end()) {
					rec.tdata.eventbssap.u16FromLAC = it->second;
					LOGDEBUG("LAC updated to %u for CI %u",
							it->second, it->first);
				}
				else {
					LOGDEBUG("LAC unknown for CI %u", rec.tdata.eventbssap.u16ToCI);
				}
			}
			switch (rec.u8Protocol) {
				case 1:
				case 2:
					/* 内部切换事件及成功切出事件 */
					++g_switch_msg;
					adapter_write(switch_adap_to, &tmp, sizeof(rec));
					break;
				default:
					break;
			}
			break;

		default:
			LOGERROR("Unknown subtype %u", pkthdr_get_subtype(ph));
			break;
		}

		free(ph);
	}

	TimerCallback(0, 0, NULL, NULL);
	itf_sd_exit();
	return 0;
}

static struct ap_framework itf_sdapp = {
	NULL,

	itf_sd_run,
	0ul,

	NULL,
	NULL,
	NULL,

	itf_sd_show_usage,
	NULL,
	itf_sd_parse_args
};

#if defined(__cplusplus)
extern "C" {
#endif

struct ap_framework *register_ap(void)
{
	return &itf_sdapp;
}

#if defined(__cplusplus)
}
#endif

