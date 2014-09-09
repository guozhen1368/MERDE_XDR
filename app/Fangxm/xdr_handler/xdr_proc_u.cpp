#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <map>
#include "syslog.h"
#include "getcfg_v2.h"
#include "myredis_async.h"
#include "xdr_handler.h"
#include "xdr_proc_u.h"
#include "xdr_sendto_merge.h"
#include "xdr_sendto_app.h"

#define map_val_t	map_u_val_t
#define LOCK(mtx)	pthread_mutex_lock(&mtx)
#define UNLOCK(mtx)	pthread_mutex_unlock(&mtx)
#define KEY_VAL		struct map_key_t, struct map_val_t 
#define REDIS_KEY_LEN	11	/* 4 + 3 + 4 */
#define REDIS_VAL_LEN	32	/* 8 + 8 + 16 */

struct map_key_t {
	byte key[XDR_U_KEY_LEN]; /* mme_ue_s1ap_id, mme_group_id, mme_code */

	bool operator < (const struct map_key_t &o) const {
		return memcmp(key, o.key, sizeof(key)) < 0;
	}
};

static map<KEY_VAL> xdr_map;
static pthread_mutex_t map_mtx = PTHREAD_MUTEX_INITIALIZER;
static struct myredis_async_handle_t *redis = NULL;
static char redis_ip[32] = "127.0.0.1";
static int  redis_port = 6379;

static int getcfg()
{
	getcfg_v2(CFGFILE, "database", "server_ip", redis_ip, GETCFG_STR);
	getcfg_v2(CFGFILE, "database", "server_port", &redis_port,
			GETCFG_INT32);
	return 0;
}

static map<KEY_VAL>::iterator add_map_key(struct map_key_t &key)
{
	struct map_val_t val;

	val.filled  = 0;
	memset(val.imsi,    0xFF, sizeof(val.imsi));
	memset(val.imei,    0xFF, sizeof(val.imei));
	memset(val.msisdn,  0xFF, sizeof(val.msisdn));
	memset(val.enb_id,  0xFF, sizeof(val.enb_id));
	memset(val.cell_id, 0xFF, sizeof(val.cell_id));
	val.last_arrived = time(NULL);
	return xdr_map.insert(xdr_map.begin(), pair<KEY_VAL>(key, val));
}

static void add_map_pkt(struct map_val_t &val, char *xdr, int bytes)
{
	list<struct xdr_pkt_t> *lst = &val.lst;
	struct xdr_pkt_t pkt;
	
	pkt.pkt_len = bytes;
	memcpy(pkt.pkt, xdr, bytes);

	lst->push_back(pkt);
	val.last_arrived = time(NULL);
}

static void fill_fields(struct map_val_t &val, char *xdr)
{
	struct xdr_hdr_t *hdr = (struct xdr_hdr_t *)xdr;

	if (val.filled == 0 && 
			xdr_is_valid_bytes(hdr->imsi, sizeof(hdr->imsi))) {
		xdr_copy_bytes(val.imsi, hdr->imsi, sizeof(hdr->imsi));
		val.filled = 1;
	}
	if (xdr_is_valid_bytes(hdr->imei, sizeof(hdr->imei)))
		xdr_copy_bytes(val.imei, hdr->imei, sizeof(hdr->imei));
	if (xdr_is_valid_bytes(&hdr->msisdn[8], 8))
		xdr_copy_bytes(val.msisdn, hdr->msisdn, sizeof(hdr->msisdn));
}

static void update_db(char *xdr)
{
	char src_key[REDIS_KEY_LEN];
	char dst_key[REDIS_KEY_LEN];
	char *dst_val = (char *)((struct xdr_hdr_t *)xdr)->imsi;

	get_xdr_src_enb_key(src_key, xdr);
	get_xdr_dst_enb_key(dst_key, xdr);
	myredis_async_command(redis, NULL, NULL, "DEL %b", 
			src_key, sizeof(src_key));
	myredis_async_command(redis, NULL, NULL, "SET %b %b",
			dst_key, sizeof(dst_key), 
			dst_val, REDIS_VAL_LEN);
}

static void send_xdr(struct map_val_t &val)
{
	xdrs2app_send(&val);

	if (xdr_is_valid_bytes(val.imsi, sizeof(val.imsi)))
		xdrs2m_send(&val);

	val.lst.clear();
}

static void *map_check_thread(void *arg)
{
	int fill_timeout = (long)arg;
	map<KEY_VAL>::iterator iter;

	while (1) {
		LOCK(map_mtx);

		for (iter = xdr_map.begin(); iter != xdr_map.end(); ) {
			if ((time(NULL) - iter->second.last_arrived) >= 
					fill_timeout) {
				send_xdr(iter->second);
				xdr_map.erase(iter++);
				continue;
			}

			iter++;
		}

		UNLOCK(map_mtx);
		sleep(fill_timeout / 2);
	}
	return NULL;
}

int xdrproc_u_start(int fill_timeout)
{
	pthread_t tid;

	getcfg();

	redis = myredis_async_init(redis_ip, redis_port);
	if (redis == NULL) {
		syslog(LOG_ERR, "myredis_async_init() failed.");
		return -1;
	}
	syslog(LOG_ERR, "myredis_async_init() success.");

	if (pthread_create(&tid, NULL, map_check_thread, 
				(void *)fill_timeout) != 0)
		return -1;
	pthread_detach(tid);
	return 0;
}

int xdrproc_u_add(char *xdr, int bytes)
{
	map<KEY_VAL>::iterator iter;
	struct map_key_t key;
	int type = get_xdr_interface((struct xdr_hdr_t *)xdr);

	if (type != XDR_TYPE_UU && type != XDR_TYPE_X2 &&
			type != XDR_TYPE_S1_MME) {
		syslog(LOG_ERR, "Not handle XDR interface in this module.");
		return -1;
	}

	get_xdr_u_key(key.key, xdr);
	
	LOCK(map_mtx);

	if (xdr_map.size() >= MAP_U_SIZE_MAX) {
		syslog(LOG_ERR, "too many note in map\n");
		UNLOCK(map_mtx);
		return -1;
	}

	iter = xdr_map.find(key);
	if (iter == xdr_map.end())
		iter = add_map_key(key);

	add_map_pkt(iter->second, xdr, bytes);

	if (type == XDR_TYPE_S1_MME)
		fill_fields(iter->second, xdr);

	if (type == XDR_TYPE_X2 && get_xdr_procedure_type(xdr) == 1)
		update_db(xdr);

	if (iter->second.filled == 1)
		send_xdr(iter->second);

	UNLOCK(map_mtx);
	return 0;
}
