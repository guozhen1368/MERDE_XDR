/*
 * (C) Copyright 2007
 * Hu Chunlin <chunlin.hu@gmail.com>
 *
 * itf.c - Simulate a packet server
 *
 */

#ifndef WIN32
#include <unistd.h>
#endif
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "os.h"
#include "apfrm.h"
#include "aplog.h"
#include "cconfig.h"
#include "pkt.h"
#include "misc.h"
#include "coding.h"
#include "cthread.h"
#include "timermgr.h"
#include "adapter.h"
#include "adapter_cs.h"
#include "adapter_ss.h"
#include "swdbus.h"

using std::map;
using std::string;

char *cfgfile = NULL;
bool  testmode = false;

void *tl = NULL;

struct adap_info {
	void *thread;

	/* Adapter or DBUS interface */
	void *adap;
	bool  is_dbus;

	/* for dbus */
	unsigned char type;
	unsigned char subtype;

	/* for adapter */
	bool  is_server;
	bool  is_focal;
	const char *cfg_section;

	unsigned long long pkts;
	unsigned long long bytes;
};

struct adap_info in[7] = {
	{ NULL, NULL, true,  PKTTYPE_CDR,   PKTTYPE_WILDCARD, false, false, NULL, 0, 0 },
	{ NULL, NULL, true,  PKTTYPE_EVENT, PKTTYPE_WILDCARD, false, false, NULL, 0, 0 },
	{ NULL, NULL, true,  PKTTYPE_KPCDR, PKTTYPE_WILDCARD, false, false, NULL, 0, 0 },
	{ NULL, NULL, false, PKTTYPE_WILDCARD, PKTTYPE_WILDCARD, true, true, "Status", 0, 0 },
	{ NULL, NULL, false, PKTTYPE_WILDCARD, PKTTYPE_WILDCARD, true, true, "Signal", 0, 0 },
	{ NULL, NULL, false, PKTTYPE_WILDCARD, PKTTYPE_WILDCARD, true, true, "Statistics", 0, 0 },
	{ NULL, NULL, false, PKTTYPE_WILDCARD, PKTTYPE_WILDCARD, true, true, "Msginfo", 0, 0 }
};

struct datatype {
public:
	datatype() {
		m_type = 0;
		m_subtype = 0;
		m_protocol = 0;
	}
	datatype(unsigned char type, unsigned char subtype, unsigned char protocol):
		m_type(type), m_subtype(subtype), m_protocol(protocol) {}

	datatype(const datatype &r) :
		m_type(r.m_type), m_subtype(r.m_subtype), m_protocol(r.m_protocol) {}

	bool operator <(const datatype& r) const
	{
		if (m_type != r.m_type)
			return (m_type < r.m_type);
		if (m_subtype != r.m_subtype)
			return (m_subtype < r.m_subtype);
		if ((m_protocol == PKTTYPE_WILDCARD) ||
			(r.m_protocol == PKTTYPE_WILDCARD))
			return false;

		return (m_protocol < r.m_protocol);
	}

public:
	unsigned char m_type;
	unsigned char m_subtype;
	unsigned char m_protocol;
};

map<struct datatype, struct adap_info *> in2out;
struct adap_info out[128];
int outnum = 0;

static void itf_show_usage(char *progname)
{
	printf("        --cfgfile <filename>: configuration file name\n");
	printf("        --testmode: testing mode (send pseudo packets)\n");
}

static int itf_parse_args(int argc, char **argv)
{
	int i = 0;

	while(i < argc) {
		if (strcmp(argv[i], "--cfgfile") == 0) {
			if (((i + 1) >= argc) || (argv[i + 1][0] == '-'))
				return -1;

			i++;
			cfgfile = argv[i];
		}
		else if (strcmp(argv[i], "--testmode") == 0) {
			i++;
			testmode = true;
		}
		else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			return -1;
		}
		i++;
	}

	return 0;
}

static void *in_thread(void *arg)
{
	struct thread_arg *targ = (struct thread_arg *)arg;
	struct adap_info  *info = (struct adap_info *)targ->arg;
	pkt_hdr *ph;
	int len;
	unsigned char type, subtype, protocol;
	map<struct datatype, struct adap_info *>::iterator it;

	TIMERLIST_ADD_TIMER_PERIODIC(tl, 300, (void *)info);
	while (ap_is_running() && targ->flag) {
		ph = (pkt_hdr *)(info->is_dbus ?
				swdbus_read(info->adap) : adapter_read(info->adap));
		if (ph) {
			len = pkthdr_get_plen(ph);
			type = pkthdr_get_type(ph);
			subtype = pkthdr_get_subtype(ph);
			protocol = pkthdr_get_protocol(ph);
			datatype dt(type, subtype, protocol);

#if defined(DEBUG)
			if (type == PKTTYPE_KPCDR) {
				LGWRDEBUG(ph, len, "Type: 0x%02x%02x", type, subtype);
			}
#endif

			if ((it = in2out.find(dt)) != in2out.end()) {
				adapter_write(it->second->adap, ph, len);
				it->second->pkts++;
				it->second->bytes += len;
			}

			info->pkts++;
			info->bytes += len;

			free(ph);
		}
		else {
			SLEEP_MS(5);
		}
	}

	return NULL;
}

static int start_adap(struct adap_info *info)
{
	if (!info || info->adap)
		return -1;

	if (info->is_dbus) {
		info->adap = swdbus_env_open(ap_is_running);
		if (info->adap == NULL) {
			LOGERROR("Failed to open dbus for type %02x/%02x",
					info->type, info->subtype);
			return -1;
		}
		if (swdbus_register_consumer(info->adap,
					info->type, info->subtype) < 0) {
			LOGERROR("Failed to register consumber for type %02x/%02x",
					info->type, info->subtype);
			swdbus_close(info->adap);
			info->adap = NULL;
			return -1;
		}
		LOGINFO("Dbus consumber of type %02x/%02x registered.",
				info->type, info->subtype);
	}
	else {
		if (info->is_server) {
			info->adap = adapter_register_ss_cfgfile(cfgfile,
					(char *)info->cfg_section, ap_is_running);
		}
		else {
			info->adap = adapter_register_cs_cfgfile(cfgfile,
					(char *)info->cfg_section, ap_is_running);
		}
		if (info->adap == NULL) {
			if (info->is_focal)
				return 0;
			LOGERROR("Failed to open adapter [%s]",  info->cfg_section);
			return -1;
		}
		adapter_open(info->adap);
		LOGINFO("Adapter [%s] registered.",  info->cfg_section);
	}

	info->thread = thread_open(in_thread, info);
	if (info->thread == NULL) {
		if (info->is_dbus)
			swdbus_close(info->adap);
		else
			adapter_close(info->adap);
		info->adap = NULL;
		return -1;
	}
	LOGINFO("Thread registered.");

	return 0;
}

static void stop_adap(struct adap_info *info)
{
	if (info->thread) {
		thread_close(info->thread);
		info->thread = NULL;
	}

	if (info->adap) {
		if (info->is_dbus)
			swdbus_close(info->adap);
		else
			adapter_close(info->adap);
		info->adap = NULL;
	}
}

static void exit_env(void)
{
	for (int i = 0; i < 7; ++i) {
		stop_adap(&in[i]);
	}

	for (int i = 0; i < outnum; ++i) {
		if (out[i].adap) {
			adapter_close(out[i].adap);
			out[i].adap = NULL;
		}
	}
}

static int get_dtype(const char *str, datatype *dt)
{
	string s[3];
	char *lstr = NULL, *p, *token, *saveptr;
	unsigned char type, subtype, protocol;
	int i;

	if (!str || ((i = strlen(str)) <= 0)) {
		LOGERROR("invalid parameter: %s", str);
		throw 1;
	}
	try {
		lstr = new char[i + 1];
	} catch (...) {
		LOGERROR("no memory to duplicate string '%s'", str);
		return -1;
	}

	strcpy(lstr, str);
	s[2] = "*";
	for (i = 0, p = lstr; i < 3; i++, p = NULL) {
		token = strtok_r(p, " ", &saveptr);
		if (token == NULL) {
			if (i < 2) {
				delete [] lstr;
				LOGERROR("'%s': needs at least 2 tokens, got %d",str,i);
				return -1;
			}
			else {
				break;
			}
		}
		s[i] = token;
	}
	delete [] lstr;

	if (pkttypes_typestr2type(s[0].c_str(), &type) < 0) {
		LOGERROR("invalid type: %s", s[0].c_str());
		return -1;
	}
	if (pkttypes_subtypestr2subtype(type, s[1].c_str(), &subtype) < 0) {
		LOGERROR("invalid subtype for %s: %s", s[0].c_str(), s[1].c_str());
		return -1;
	}
	if (pkttypes_protocolstr2protocol(type, subtype,
				s[2].c_str(), &protocol) < 0) {
		LOGERROR("invalid protocol for %s %s: %s",
				s[0].c_str(), s[1].c_str(), s[2].c_str());
		return -1;
	}

	*dt = datatype(type, subtype, protocol);
	LOGINFO("%s %s %s: 0x%02x%02x%02x",
			s[0].c_str(), s[1].c_str(), s[2].c_str(), type, subtype, protocol);

	return 0;
}

static int load_out(void)
{
	unsigned long cfghd;

	cfghd = CfgInitialize(cfgfile);
	if (cfghd != 0ul) {
		int         adapters, dtypes, servers, servertype;
		const char *sname = "Adapter.Setting";
		const char *dtname = "data type";
		char        value[1024], tmpvalue[1152], cfgstr[4096];

		adapters = CfgGetCount(cfghd, sname, NULL, 0);
		if (adapters < 1) {
			LOGERROR("Section %s: Not found.", sname);
			return -1;
		}
		LOGINFO("Section %s: %d", sname, adapters);

		for (int idx = 1; idx <= adapters; idx++) {
			dtypes = CfgGetCount(cfghd, sname, dtname, idx);
			if (dtypes < 1) {
				LOGERROR("Section %s[%d]: %s not found.", sname, idx, dtname);
				return -1;
			}
			LOGINFO("Section %s[%d]: %s: %d.", sname, idx, dtname, dtypes);

			/* server list */
			servers = CfgGetCount(cfghd, sname, "server", idx);
			if (servers < 1) {
				LOGERROR("Section %s[%d]: server not found.", sname, idx);
				return -1;
			}
			LOGINFO("Section %s[%d]: server: %d.", sname, idx, servers);

			strcpy(cfgstr, "[a]\n");
			for (int svr = 1; svr <= servers; ++svr) {
				if (CfgGetValue(cfghd, sname, "server", value, svr, idx) == -1){
					LOGERROR("Section %s[%d]: Loading server[%d] failed.",
							sname, idx, svr);
					return -1;
				}
				LOGINFO("Section %s[%d]: Loading server[%d]: %s",
						sname, idx, svr, value);
				sprintf(tmpvalue, "server=%s\n", value);
				strcat(cfgstr, tmpvalue);
			}

			servertype = 0;
			if (CfgGetValue(cfghd, sname, "server type", value, 1, idx) == 0) {
				if (STRCASECMP(0, value, "ss") == 0) {
					servertype = 1;
					LOGINFO("Section %s[%d]: Loading server type %s.",
							sname, idx, value);
				}
				else {
					LOGINFO("Section %s[%d]:"
							" Loading default server type cs[%s].",
							sname, idx, value);
				}
			}

			/* register adapter */
			if (servertype) {
				out[outnum].adap = adapter_register_ss_cfgstr(cfgstr,
						(char *)"a", ap_is_running);
			}
			else {
				out[outnum].adap = adapter_register_cs_cfgstr(cfgstr,
						(char *)"a", ap_is_running);
			}
			if (out[outnum].adap == NULL) {
				LOGERROR("Section %s[%d]: Register server %s failed.",
						sname, idx, value);
				return -1;
			}
			out[outnum].type = (unsigned char)idx;
			adapter_open(out[outnum].adap);
			TIMERLIST_ADD_TIMER_PERIODIC(tl, 300, (void *)&out[outnum]);
			LOGINFO("Section %s[%d]: Server %s registered at %p.",
					sname, idx, value, out[outnum].adap);

			/* corresponding data types */
			for (int dt = 1; dt <= dtypes; dt++) {
				datatype us;

				if (CfgGetValue(cfghd, sname, dtname, value, dt, idx) == -1) {
					LOGERROR("Section %s[%d]: Loading data type %d failed.",
							sname, idx, dt);
					adapter_close(out[outnum].adap);
					out[outnum].adap = NULL;
					return -1;
				}

				if (get_dtype((const char *)value, &us) < 0) {
					LOGERROR("Section %s[%d]: Invalid data type[%d]: %s",
							sname, idx, dt, value);
					adapter_close(out[outnum].adap);
					out[outnum].adap = NULL;
					return -1;
				}
				in2out[us] = &out[outnum];
				LOGINFO("Section %s[%d]: data type[%d]: %s mapped to %p",
							sname, idx, dt, value, out[outnum].adap);
			}

			++outnum;
		}

		CfgInvalidate(cfghd);
		return 0;
	}

	return -1;
}

static int init_env(void)
{
	for (int i = 0; i < 128; ++i)
		memset(&out[i], 0, sizeof(out[i]));

	if (load_out() < 0)
		return -1;

	for (int i = 0; i < 7; ++i) {
		if (start_adap(&in[i]) < 0)
			return -1;
	}

	return 0;
}

void itf_cb(unsigned int s, unsigned int ns, void *data, void *arg)
{
	struct adap_info *info = (struct adap_info *)data;

	if (info) {
		if (info->is_dbus) {
			LOG("ITF: From dbus(%02x%02x): %llu pkts, %llu bytes",
					info->type, info->subtype, info->pkts, info->bytes);
		}
		else if (info->cfg_section) {
			LOG("ITF: From adapter(%s): %llu pkts, %llu bytes",
					info->cfg_section, info->pkts, info->bytes);
		}
		else {
			LOG("ITF: To adapter(%u): %llu pkts, %llu bytes",
					info->type, info->pkts, info->bytes);
		}
	}
}

static int itf_run(long instance, unsigned long data)
{
	unsigned char  buffer[128];
	pkt_hdr       *ph = (pkt_hdr *)buffer;

	memset(buffer, 0, sizeof(buffer));
	pkthdr_set_sync(ph);
	pkthdr_set_type(ph, 0x40);
	memcpy(buffer + sizeof(pkt_hdr), "270071300001", 12);

	tl = TIMERLIST_CREATE(TIMERMGR_SOURCE_LOCAL, itf_cb, NULL, "ITF");
	if (init_env() == 0) {
		while (ap_is_running()) {
			SLEEP_S(1);
			if (!testmode)
				continue;

			for (unsigned char subtype = 0x01; subtype <= 0x03; ++subtype) {
				map<struct datatype, struct adap_info *>::iterator it;
				datatype dt(0x40, subtype, PKTTYPE_WILDCARD);
				struct timeval t;

				gettimeofday(&t, NULL);
				if ((it = in2out.find(dt)) != in2out.end()) {
					int len;
					unsigned char *pbuf;

					pbuf = buffer + sizeof(pkt_hdr) + 12;
					if (subtype == 0x01) {
						pkthdr_set_subtype(ph, 0x01);
						len = sizeof(pkt_hdr) + 12;
					}
					else if (subtype == 0x02) {
						static unsigned char status, cpu = 1, mem = 1, lost;
						static unsigned long long ibytes, obytes;

						pkthdr_set_subtype(ph, 0x02);
						len = sizeof(pkt_hdr) + 33;

						CODING_8(pbuf, 0xff);
						CODING_8(pbuf, status);
						status = 1 - status;
						CODING_8(pbuf, cpu++);
						if (cpu == 101)
							cpu = 1;
						CODING_8(pbuf, mem++);
						if (mem == 101)
							mem = 1;
						CODING_8(pbuf, lost++);
						CODING_64(pbuf, ibytes);
						ibytes += 0x100000;
						CODING_64(pbuf, obytes);
						obytes += 0x100000;
					}
					else {
						static unsigned char alarm;

						pkthdr_set_subtype(ph, 0x03);
						len = sizeof(pkt_hdr) + 23;

						CODING_8(pbuf, 0xff);
						CODING_8(pbuf, alarm);
						alarm = 1 - alarm;
						CODING_32(pbuf, t.tv_sec);
						CODING_32(pbuf, t.tv_usec / 1000);
						CODING_8(pbuf, 3);
					}
					pkthdr_set_ts(ph, (unsigned int)t.tv_sec,
							(unsigned int)t.tv_usec * 1000);
					pkthdr_set_plen(ph, len);

#if defined(DEBUG)
					LGWRDEBUG(ph, len, "Type: 0x40%02x", subtype);
#endif
					adapter_write(it->second->adap, ph, len);
					it->second->pkts++;
					it->second->bytes += len;
				}
			}
		}
	}
	exit_env();
	TIMERLIST_RELEASE(tl);

	return 0;
}

static struct ap_framework itfapp = {
	NULL,
	itf_run,
	0ul,
	NULL,
	NULL,
	NULL,
	itf_show_usage,
	NULL,
	itf_parse_args,
};

#if defined(__cplusplus)
extern "C"
{
#endif

struct ap_framework *register_ap(void)
{
	return &itfapp;
}

#if defined(__cplusplus)
}
#endif
