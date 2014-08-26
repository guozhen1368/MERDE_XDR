/*
 * (C) Copyright 2008
 * Hu Chunlin <chunlin.hu@gmail.com>
 *
 * adapter_cs.h - A brief description goes here.
 *
 * There must be a configuration section to run this adapter. An
 * example config is listed below. Name of the configuration section
 * is specified by parameter 'section'.
 *

[Adapter.ClientSocket]
# FORMAT: server = IP,Port,RxBufSize,TxBufSize[,Flag[,Flag[...]]
#
# RxBufSize,TxBfSize: in KBytes
# Flag: payload - Only payload will be sent to/received from peers.
#
# Example: server = 192.168.7.100,8888,256,262144,payload
server = 192.168.1.20,7121,1024,4096
server = 192.168.1.21,7121,1024,4096

disk cache = archive

[archive]
path = /tmp
prefix = cache_
datefmt = %04d%02d%02d%02d%02d
secfmt = %02d
suffix = .dat
tempname = NO_SUFFIX
period = 0
filesz = 10
concurrent = 1
disabled = no
hsl dlt = 0x93

 */

#ifndef _HEAD_ADAPTER_CS_1B9D7B61_10298EC9_610D34B6_H
#define _HEAD_ADAPTER_CS_1B9D7B61_10298EC9_610D34B6_H

#include "cssockdef.h"

#ifndef DLL_APP
#ifdef WIN32
#ifdef _USRDLL
#define DLL_APP _declspec(dllexport)
#else
#define DLL_APP _declspec(dllimport)
#endif
#else
#define DLL_APP
#endif
#endif

#define ADAPTER_CS_IOCTL_CLOSE_SOCKET 1
#define ADAPTER_CS_IOCTL_CLEAR_RQ     2
#define ADAPTER_CS_IOCTL_CLEAR_TQ     3

struct adapter_cs_ioctl {
	char *ip;
	int   port;
};

#if defined(__cplusplus)
extern "C" {
#endif

DLL_APP void *adapter_register_cs_c_i(unsigned long cfghd,
		char *section, int sid,
		int (*running)(void), int (*pktsync)(void *buf, int size));
#define adapter_register_cs_c(cfghd, section, running, pktsync) \
	adapter_register_cs_c_i(cfghd, section, 1, running, pktsync)
#define adapter_register_cs(cfghd, section, running) \
	adapter_register_cs_c_i(cfghd, section, 1, running, NULL)
#define adapter_register_cs_i(cfghd, section, sid, running) \
	adapter_register_cs_c_i(cfghd, section, sid, running, NULL)

DLL_APP void *adapter_register_cs_cfgfile_c_i(char *cfgfile,
		char *section, int sid,
		int (*running)(void), int (*pktsync)(void *buf, int size));
#define adapter_register_cs_cfgfile_c(cfgfile, section, running, pktsync) \
	adapter_register_cs_cfgfile_c_i(cfgfile, section, 1, running, pktsync)
#define adapter_register_cs_cfgfile(cfgfile, section, running) \
	adapter_register_cs_cfgfile_c_i(cfgfile, section, 1, running, NULL)
#define adapter_register_cs_cfgfile_i(cfgfile, section, sid, running) \
	adapter_register_cs_cfgfile_c_i(cfgfile, section, sid, running, NULL)

DLL_APP void *adapter_register_cs_cfgstr_c_i(char *cfgstr,
		char *section, int sid,
		int (*running)(void), int (*pktsync)(void *buf, int size));
#define adapter_register_cs_cfgstr_c(cfgstr, section, running, pktsync) \
	adapter_register_cs_cfgstr_c_i(cfgstr, section, 1, running, pktsync)
#define adapter_register_cs_cfgstr(cfgstr, section, running) \
	adapter_register_cs_cfgstr_c_i(cfgstr, section, 1, running, NULL)
#define adapter_register_cs_cfgstr_i(cfgstr, section, sid, running) \
	adapter_register_cs_cfgstr_c_i(cfgstr, section, sid, running, NULL)

/* These two functions must be called after adapter_register_cs* and
 * before adapter_open.
 */
DLL_APP void adapter_cs_setconncb(void *adap, CSSOCK_CONN_CB conncb, void *arg);
DLL_APP void adapter_cs_setdisccb(void *adap, CSSOCK_DISC_CB disccb, void *arg);

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef _HEAD_ADAPTER_CS_1B9D7B61_10298EC9_610D34B6_H */
