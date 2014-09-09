#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "syslog.h"
#include "getcfg_v2.h"
#include "tcpconn.h"
#include "set_keepalive.h"
#include "xdr_pkt.h"
#include "xdr_sendto_app.h"
#include "xdr_handler.h"
#include "xdr_proc_u.h"
#include "xdr_proc_s.h"

#define KEEPALIVE_IDEL	30	/* start keeplives after period(seconds) */
#define KEEPALIVE_CNT	5	/* number of keepalives before death */
#define KEEPALIVE_INTV	3	/* interval between keepalives */
#define INVALID_SOCKET	-1
#define LOCK(mtx)	pthread_mutex_lock(&mtx)
#define UNLOCK(mtx)	pthread_mutex_unlock(&mtx)
#define PKT_HDR_SYNC	0x7E5A
#define PKT_TYPE_XDR	0x09
#define PKT_SINGLE_XDR	2

#pragma pack(1)
struct app_pkt_hdr_t {
	unsigned short sync;
	unsigned short len;
	unsigned char  type;
	unsigned char  subtype;
	unsigned char  protocol;
	unsigned char  sc;
	unsigned short device;
	unsigned short channel;
	unsigned int   ts_s;
	unsigned int   ts_ns;
};
#pragma pack()

struct app_pkt_t {
	struct app_pkt_hdr_t hdr;
	unsigned char xdr_type;
	unsigned char xdr[XDR_LEN_MAX];
};

static int handle_mode;
static char server_ip[32] = {0};
static int  server_port = -1;
static int  server_sock = INVALID_SOCKET;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static struct app_pkt_t app_pkt;

static int getcfg()
{
	const char *sname = "application server";

	if (getcfg_v2(CFGFILE, sname, "server_ip", server_ip,
				GETCFG_STR) != 0) {
		syslog(LOG_ERR, "application server ip not defined.");
		return -1;
	}
	if (getcfg_v2(CFGFILE, sname, "server_port", &server_port,
				GETCFG_INT32) != 0) {
		syslog(LOG_ERR, "application server port not defined.");
		return -1;
	}
	return 0;
}

static void dummy_recv_loop(int sock)
{
	char buff[1024];
	int bytes;

	while (1) {
		bytes = recv(sock, buff, sizeof(buff), 0);
		if (bytes == -1 && errno == EINTR)
			continue;
		if (bytes <= 0)
			break;
	}
}

static void *keepalive_thread(void *arg)
{
	int sock;
	int print_errmsg = 1;

	while (1) {
		sock = tcpconn(server_ip, server_port, 5);
		if (sock == -1) {
			if (print_errmsg == 1) {
				syslog(LOG_ERR, "connect app server failed.");
				print_errmsg = 0;
			}
			sleep(5);
			continue;
		}
		syslog(LOG_INFO, "connect app server success.");
		set_keepalive(sock, KEEPALIVE_IDEL, KEEPALIVE_CNT, 
				KEEPALIVE_INTV);

		LOCK(mtx);
		server_sock = sock;
		UNLOCK(mtx);

		dummy_recv_loop(server_sock);

		LOCK(mtx);
		close(server_sock);
		server_sock = INVALID_SOCKET;
		UNLOCK(mtx);
		syslog(LOG_INFO, "disconnect from app server.");

		print_errmsg = 1;
		sleep(5);
	}

	return NULL;
}

static inline int set_pkt_len(struct app_pkt_hdr_t *hdr, int xdr_len)
{
	int pkt_len = sizeof(struct app_pkt_hdr_t) + 1 + xdr_len;
	int filler;

	filler = 4 - (pkt_len % 4);
	if (filler >= 4)
		filler = 0;

	hdr->sc &= 0xFC;
	hdr->sc |= ((unsigned char)filler & 0x03);
	hdr->len = htons((unsigned short)((pkt_len + filler) / 4));
	return pkt_len;
}

static inline int fill_pkt_u(struct map_u_val_t *pval, char *xdr, int xdr_len)
{
	byte *imsi = ((struct xdr_hdr_t *)xdr)->imsi;
	memcpy(imsi, pval->imsi, 32);
	memcpy(app_pkt.xdr, xdr, xdr_len);
	return set_pkt_len(&app_pkt.hdr, xdr_len);
}

static inline int fill_pkt_s(struct map_s_val_t *pval, char *xdr, int xdr_len)
{
	byte *imei = ((struct xdr_hdr_t *)xdr)->imei;
	memcpy(imei, pval->imei, 24);
	memcpy(app_pkt.xdr, xdr, xdr_len);
	return set_pkt_len(&app_pkt.hdr, xdr_len);
}

static int send_xdr_u(void *val)
{
	struct map_u_val_t *pval = (struct map_u_val_t *)val;
	list<struct xdr_pkt_t>::iterator iter;
	struct xdr_pkt_t *pkt;
	int bytes;

	for (iter = pval->lst.begin(); iter != pval->lst.end(); iter++) {
		pkt = (struct xdr_pkt_t *)(&(*iter));
		bytes = fill_pkt_u(pval, pkt->pkt, pkt->pkt_len);
		send(server_sock, &app_pkt, bytes, 0);
		printf("send to app server\n");
	}
	return 0;
}

static int send_xdr_s(void *val)
{
	struct map_s_val_t *pval = (struct map_s_val_t *)val;
	list<struct xdr_pkt_t>::iterator iter;
	struct xdr_pkt_t *pkt;
	int bytes;

	for (iter = pval->lst.begin(); iter != pval->lst.end(); iter++) {
		pkt = (struct xdr_pkt_t *)(&(*iter));
		bytes = fill_pkt_s(pval, pkt->pkt, pkt->pkt_len);
		send(server_sock, &app_pkt, bytes, 0);
		printf("send to app server\n");
	}
	return 0;
}

int xdrs2app_start(int mode)
{
	pthread_t tid;

	handle_mode = mode;
	if (getcfg() != 0)
		return -1;

	app_pkt.hdr.sync = htons(PKT_HDR_SYNC);
	app_pkt.hdr.type = PKT_TYPE_XDR;
	app_pkt.xdr_type = PKT_SINGLE_XDR;

	if (pthread_create(&tid, NULL, keepalive_thread, NULL) != 0) {
		syslog(LOG_ERR, "create xdrs2app keepalive_thread failed.");
		return -1;
	}
	pthread_detach(tid);
	return 0;
}

int xdrs2app_send(void *val)
{
	int ret = -1;
	LOCK(mtx);

	if (server_sock != INVALID_SOCKET) {
		if (handle_mode == XDR_MODE_U)
			ret = send_xdr_u(val);
		else if (handle_mode == XDR_MODE_S)
			ret = send_xdr_s(val);
	}

	UNLOCK(mtx);
	return ret;
}
