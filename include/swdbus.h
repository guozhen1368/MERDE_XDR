/*
 * (C) Copyright 2009
 * Beijing HLYT Technology Co., Ltd.
 *
 * swdbus.h - A brief description to describe this file.
 *
 */

#ifndef _HEAD_SWDBUS_6AF41A37_13083F52_2DFB924C_H
#define _HEAD_SWDBUS_6AF41A37_13083F52_2DFB924C_H

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

#if defined(__cplusplus)
extern "C" {
#endif

DLL_APP void *swdbus_open(char *ip, int port, int (*running)(void));
DLL_APP void *swdbus_env_open(int (*running)(void));
DLL_APP int  swdbus_get_major_minor(unsigned char *major, unsigned char *minor);
DLL_APP int   swdbus_register_producer(void *dbus,
				unsigned char type, unsigned char subtype,
				int (*compare)(void *ph, void *filter));
DLL_APP int   swdbus_register_producer_ex(void *dbus,
				unsigned char major, unsigned char minor,
				unsigned char type, unsigned char subtype,
				int (*compare)(void *ph, void *filter));
DLL_APP void swdbus_set_producer_pre_compare(void *bus,
				void *(*pre_compare)(void *data));
DLL_APP void swdbus_set_producer_post_compare(void *bus,
				void (*post_compare)(void *hd));
DLL_APP int   swdbus_register_consumer(void *dbus, 
				unsigned char type, unsigned char subtype);
DLL_APP int   swdbus_register_consumer_ex(void *dbus, 
				unsigned char major, unsigned char minor,
				unsigned char type, unsigned char subtype);
DLL_APP int   swdbus_update_filter(void *dbus, void *filter);

DLL_APP void *swdbus_read(void *dbus);
DLL_APP int   swdbus_write(void *dbus, void *data, int len);
DLL_APP int   swdbus_write_ex(void *dbus, void *data, int len, void *ex);
DLL_APP int   swdbus_write_to(void *dbus,
				void *data, int len, char *ip, int port);
DLL_APP void  swdbus_get_mem(void *bus,
				unsigned int *pkt, unsigned int *size, unsigned int *lastround);
DLL_APP int   swdbus_unregister(void *dbus);
DLL_APP void  swdbus_close(void *dbus);

DLL_APP void  swdbus_set_keepalive_interval(void *dbus, int interval);

DLL_APP void *swdbus_read_cmd(void *dbus, unsigned int *cmdtype);
DLL_APP void *swdbus_read_cmd_from(void *dbus, unsigned int *cmdtype,
				char *ip, int *port);

DLL_APP int   swdbus_send_cmd(void *dbus, void *cmd);
DLL_APP int   swdbus_send_cmd_to(void *dbus, void *cmd, char *ip, int port);

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef _HEAD_SWDBUS_6AF41A37_13083F52_2DFB924C_H */
