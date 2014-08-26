/*
 * (C) Copyright 2008
 * Hu Chunlin <chunlin.hu@gmail.com>
 *
 * cssockdef.h - A brief description goes here.
 *
 */

#ifndef _HEAD_CSSOCKDEF_1A2A06A1_2805035D_14B397B6_H
#define _HEAD_CSSOCKDEF_1A2A06A1_2805035D_14B397B6_H

#define CSSOCK_RXALIGN_BUFSZ 131072
#define CSSOCK_TXTMPBUF_SIZE 131072

#define CSSOCK_PAYLOAD_ONLY        0X00000001 /* Obsolete */
#define CSSOCK_CUSTOMIZED_PKTSYNC  0X00000002
#define CSSOCK_NO_HEARTBEAT        0X00000010
#define CSSOCK_BLOCK_MODE          0X00000020

typedef void (*CSSOCK_CONN_CB)(void *arg,
		char *lip, int lport, char *rip, int rport);
typedef void (*CSSOCK_DISC_CB)(void *arg,
		char *lip, int lport, char *rip, int rport, char *msg);

typedef void (*CSSOCK_STAT_CB)(char *lip, int lport, char *rip, int rport,
		unsigned long long pkts, unsigned long long bytes,
		unsigned long long lostpkts, unsigned long long lostbytes, void *arg);


#endif /* #ifndef _HEAD_CSSOCKDEF_1A2A06A1_2805035D_14B397B6_H */
