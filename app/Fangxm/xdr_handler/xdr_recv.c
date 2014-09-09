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
#include "tcplsn.h"
#include "set_keepalive.h"
#include "recvn.h"

#include "xdr_handler.h"
#include "xdr_recv.h"
#include "xdr_proc.h"
#include "xdr_pkt.h"

#define KEEPALIVE_IDEL	30	/* start keeplives after period(seconds) */
#define KEEPALIVE_CNT	5	/* number of keepalives before death */
#define KEEPALIVE_INTV	3	/* interval between keepalives */

static int listen_port;
static uint64_t pkt_count = 0;

struct cli_info_t {
	int sock;
	char ip[32];
};

static int getcfg(int handle_mode)
{
	const char *section = handle_mode == XDR_MODE_U ?
		"xdru_recv" : "xdrs_recv";

	if (getcfg_v2(CFGFILE, section, "listen_port", &listen_port,
				GETCFG_INT32) != 0) {
		syslog(LOG_ERR, "listen port not defined.");
		return -1;
	}

	return 0;
}

static void *recv_thread(void *arg)
{
	struct cli_info_t info;
	int sock;
	int hdr_len = sizeof(struct xdr_hdr_t);
	int pkt_len;
	int body_len;
	char buff[XDR_LEN_MAX];
	int bytes;
	char content[1024];

	memcpy(&info, arg, sizeof(struct cli_info_t));
	sock = info.sock;
	free(arg);

	syslog(LOG_INFO, "[CLIENT  IN] %s", info.ip);

	while (1) {
		/* receive xdr header */
		bytes = recvn(sock, buff, hdr_len, 0);
		if (bytes != hdr_len)
			break;

		/* receive xdr body */
		pkt_len = get_xdr_pkt_len((struct xdr_hdr_t *)buff);
		if (pkt_len <= hdr_len || pkt_len > XDR_LEN_MAX)
			break;
		body_len = pkt_len - hdr_len;
		bytes = recvn(sock, &buff[hdr_len], body_len, 0);
		if (bytes != body_len)
			break;

		if (xdr_pkt_is_valid(buff, pkt_len) != 0) {
			syslog(LOG_ERR, "invalid XDR packet.");
			continue;
		}

		xdr_dump(buff, content, sizeof(content));
		syslog(LOG_DEBUG, "packet no: %llu\n%s",
				__sync_add_and_fetch(&pkt_count, 1), content);
		xdrproc_add(buff, pkt_len);
	}

	syslog(LOG_INFO, "[CLIENT OUT] %s", info.ip);
	close(sock);
	return NULL;
}

static void *accept_thread(void *arg)
{
	int lsn_sock = (long)arg;
	int cli_sock;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	struct cli_info_t *info;
	pthread_t tid;

	while (1) {
		cli_sock = accept(lsn_sock, (struct sockaddr *)&addr, &len);
		if (cli_sock < 0) {
			syslog(LOG_ERR, "accept error:%s", strerror(errno));
			continue;
		}

		set_keepalive(cli_sock, KEEPALIVE_IDEL, KEEPALIVE_CNT, 
				KEEPALIVE_INTV);
		info = (struct cli_info_t *)malloc(sizeof(struct cli_info_t));
		if (info == NULL) {
			syslog(LOG_ERR, "allocate cli_info failed");
			close(cli_sock);
			continue;
		}

		info->sock = cli_sock;
		snprintf(info->ip, sizeof(info->ip), inet_ntoa(addr.sin_addr));

		if (pthread_create(&tid, NULL, recv_thread, info) != 0) {
			syslog(LOG_ERR, "create recv_thread failed");
			close(cli_sock);
		} else {
			pthread_detach(tid);
		}
	}
	return NULL;
}

int xdrrecv_start(int handle_mode)
{
	int sock;
	pthread_t tid;

	if (getcfg(handle_mode) != 0)
		return -1;

	sock = tcplsn(listen_port);
	if (sock == -1) {
		syslog(LOG_ERR, "create listen socket failed.");
		return -1;
	}
	syslog(LOG_INFO, "listen on port %d to recv xdr.", listen_port);

	if (pthread_create(&tid, NULL, accept_thread, (void *)sock) != 0) {
		syslog(LOG_ERR, "create accept_thread failed.");
		return -1;
	}
	pthread_detach(tid);
	return 0;
}
