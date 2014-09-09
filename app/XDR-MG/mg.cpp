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
#include "adapter_if6cli.h"
#include "adapter_ss.h"

#include "mg_compose.h"
#include "redis_help.h"

char *cfgfile = NULL;
static void *tl = NULL;
static XdrCompose *xdr_compose = NULL;

struct thread_info{
	void *thread;
	char ip[64];
	int port;
	RedisHelp *redis;
};

struct adap_info {
	/* Adapter interface */
	void *adap;

	/* for adapter */
	bool  is_server;
	bool  is_focal;
	const char *cfg_section;

	unsigned long long pkts;
	unsigned long long bytes;
};

struct adap_info adap_in = { NULL, true, false, "Signal", 0, 0 };
struct adap_info adap_out = { NULL, false, false, "sdtp", 0, 0 };

struct thread_info redis_in = {NULL, "127.0.0.1", 6379, NULL}; 


static void mg_show_usage(char *progname)
{
	printf("        --cfgfile <filename>: configuration file name\n");
	//printf("        --testmode: testing mode (send pseudo packets)\n");
}

static int mg_parse_args(int argc, char **argv)
{
	int i = 0;

	while(i < argc) {
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

	return 0;
}

static void *in_thread(void *arg)
{
	struct thread_arg *targ = (struct thread_arg *)arg;
	struct thread_info  *info = (struct thread_info *)targ->arg;

	RedisHelp *redis = info->redis;
	if (redis == NULL) {
		LOGERROR("in_thread redis is null.");
		return NULL;
	}

	redis->init();
	redis->start();

	return NULL;
}

static void mg_cb(unsigned int s, unsigned int ns,
	void *data, void *arg)
{
	struct adap_info *info = (struct adap_info *)data;

	if (info) {
		if (info->cfg_section) {
			LOG("mg: From adapter[%s]: %llu pkts, %llu bytes",
				info->cfg_section, info->pkts, info->bytes);
		}
	}
}

static int mg_pkt(void *buf, int size)
{
	uint16_t len = 0;

	if (size > 2)
	{
		memcpy(&len, buf, sizeof(uint16_t));
		len = be16toh(len);
	}

	if (size >= len && len >0)
	{
		return len;
	}

	return 0;
}

static int start_adap_dbus(struct adap_info *info)
{
	if (!info || info->adap)
		return -1;

	if (info->is_server) {
		info->adap = adapter_register_ss_cfgfile_c(cfgfile,
			(char *)info->cfg_section, ap_is_running, mg_pkt);
	}
	else{
		info->adap = adapter_register_if6cli_cfgfile(cfgfile,
			(char *)info->cfg_section);
	}

	if (info->adap == NULL) {
		LOGERROR("Failed to open adapter [%s]",  info->cfg_section);
		return -1;
	}
	adapter_open(info->adap);


	return 0;
}

static void stop_adap(struct adap_info *info)
{
	if (info->adap) {
		adapter_close(info->adap);
		info->adap = NULL;
	}
}

static void exit_env(void)
{
	stop_adap(&adap_in);
	stop_adap(&adap_out);

	if (xdr_compose) {
		delete xdr_compose;
		xdr_compose = NULL;
	}

	if (redis_in.thread)
	{
		thread_close(redis_in.thread);
		redis_in.thread = NULL;
	}

	if (redis_in.redis)
	{
		delete redis_in.redis;
		redis_in.redis = NULL;
	}

	if (tl) {
		TIMERLIST_RELEASE(tl);
	}
}

static int init_env(void)
{
	unsigned long cfghd;

	tl = TIMERLIST_CREATE(TIMERMGR_SOURCE_LOCAL, mg_cb,
		NULL, "mg_main");
	if (tl == NULL) {
		LOGERROR("Create timerlist fail.");
		return -1;
	}

	cfghd = CfgInitialize(cfgfile);
	if (cfghd == 0ul) {
		LOGINFO("Load config file fail.");
		return -1;
	}

	int port = CfgGetValueInt(cfghd, "Redis", "port", 1, 1);
	redis_in.port = port;
	CfgGetValue(cfghd, "Redis", "ip", redis_in.ip, 1, 1);

	RedisHelp *redis = new RedisHelp(redis_in.ip, redis_in.port);
	if (redis == NULL) {
		CfgInvalidate(cfghd);
		LOGERROR("new redis fail.");
		return -1;
	}
	
	redis_in.redis = redis;
	redis_in.thread = thread_open(in_thread, &redis_in);
	if (redis_in.thread == NULL) {
		CfgInvalidate(cfghd);
		LOGERROR("new thread redis fail.");
		return -1;
	}
	LOGINFO("Thread registered.");

	SLEEP_MS(1000);
	try {
		xdr_compose = new XdrCompose(1, cfghd, redis_in.redis);
	} catch(...) {
		xdr_compose = NULL;
		CfgInvalidate(cfghd);
		LOGERROR("new CCDRCSFBCompose() fail.");
		return -1;
	}

	if (!xdr_compose->init())
	{
		return -1;
	}

	if (start_adap_dbus(&adap_in) < 0) {
		CfgInvalidate(cfghd);
		return -1;
	}

	if (start_adap_dbus(&adap_out) < 0) {
		CfgInvalidate(cfghd);
		return -1;
	}

	TIMERLIST_ADD_TIMER_PERIODIC(tl, 60, (void *)&adap_in);
	TIMERLIST_ADD_TIMER_PERIODIC(tl, 60, (void *)&adap_out);


	CfgInvalidate(cfghd);

	return 0;
}

static int mg_run(long instance, unsigned long data)
{
	pkt_hdr *ph;
	int len;
	int i = 0;

	if (init_env() != 0) {
		exit_env();
		return -1;
	}

	while (ap_is_running()) {
		if (i++ == 1000)
		{
			xdr_compose->runTimer();
			i = 0;
		}
		
		ph = (pkt_hdr *)adapter_read(adap_in.adap);
		if (ph == NULL) {
			SLEEP_MS(5);
			continue;
		}

		len = pkthdr_get_dlen(ph);

		++(adap_in.pkts);
		adap_in.bytes += len;

		xdr_compose->deal_xdr(ph);

		//free(ph);
	}

	redis_in.redis->stop();
	//sleep(1);

	mg_cb(0, 0, (void *)&adap_in, NULL);
	mg_cb(0, 0, (void *)&adap_out, NULL);


	exit_env();

	return 0;
}

static struct ap_framework mgapp = {
	NULL,
	mg_run,
	0ul,
	NULL,
	NULL,
	NULL,
	mg_show_usage,
	NULL,
	mg_parse_args,
};

#if defined(__cplusplus)
extern "C"
{
#endif

struct ap_framework *register_ap(void)
{
	return &mgapp;
}

#if defined(__cplusplus)
}
#endif
