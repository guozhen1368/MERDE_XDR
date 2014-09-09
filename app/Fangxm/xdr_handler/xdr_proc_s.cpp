#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <map>
#include "syslog.h"
#include "xdr_proc_s.h"
#include "xdr_sendto_merge.h"
#include "xdr_sendto_app.h"

#define map_val_t	map_s_val_t
#define LOCK(mtx)	pthread_mutex_lock(&mtx)
#define UNLOCK(mtx)	pthread_mutex_unlock(&mtx)
#define KEY_VAL		struct map_key_t, struct map_val_t 

struct map_key_t {
	byte key[XDR_S_KEY_LEN];	/* imsi */

	bool operator < (const struct map_key_t &o) const {
		return memcmp(key, o.key, sizeof(key)) < 0;
	}
};

static map<KEY_VAL> xdr_map;
static pthread_mutex_t map_mtx = PTHREAD_MUTEX_INITIALIZER;

static map<KEY_VAL>::iterator add_map_key(struct map_key_t &key)
{
	struct map_val_t val;

	val.filled  = 0;
	memset(val.imei,    0xFF, sizeof(val.imei));
	memset(val.msisdn,  0xFF, sizeof(val.msisdn));
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

	if (xdr_is_valid_bytes(hdr->imei, sizeof(hdr->imei)))
		xdr_copy_bytes(val.imei, hdr->imei, sizeof(hdr->imei));
	if (xdr_is_valid_bytes(&hdr->msisdn[8], 8))
		xdr_copy_bytes(val.msisdn, hdr->msisdn, sizeof(hdr->msisdn));

	if (xdr_is_valid_bytes(val.imei, sizeof(val.imei)) &&
			xdr_is_valid_bytes(&val.msisdn[8], 8))
		val.filled = 1;
}

static void send_xdr(const struct map_key_t &key, struct map_val_t &val)
{
	xdrs2app_send(&val);

	if (xdr_is_valid_bytes((byte *)key.key, sizeof(key.key)))
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
				send_xdr(iter->first, iter->second);
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

int xdrproc_s_start(int fill_timeout)
{
	pthread_t tid;

	if (pthread_create(&tid, NULL, map_check_thread, 
				(void *)fill_timeout) != 0)
		return -1;
	pthread_detach(tid);
	return 0;
}

int xdrproc_s_add(char *xdr, int bytes)
{
	map<KEY_VAL>::iterator iter;
	struct map_key_t key;
	int type = get_xdr_interface((struct xdr_hdr_t *)xdr);

	if (type != XDR_TYPE_SGS && type != XDR_TYPE_S6A &&
			type != XDR_TYPE_S1_MME) {
		syslog(LOG_ERR, "Not handle XDR interface in this module.");
		return -1;
	}

	get_xdr_s_key(key.key, xdr);

	LOCK(map_mtx);

	if (xdr_map.size() >= MAP_S_SIZE_MAX) {
		syslog(LOG_ERR, "too many note in map\n");
		UNLOCK(map_mtx);
		return -1;
	}

	iter = xdr_map.find(key);
	if (iter == xdr_map.end())
		iter = add_map_key(key);

	if (type != XDR_TYPE_S1_MME)
		add_map_pkt(iter->second, xdr, bytes);

	if (type == XDR_TYPE_S1_MME && iter->second.filled == 0)
		fill_fields(iter->second, xdr);

	if (iter->second.filled == 1)
		send_xdr(iter->first, iter->second);

	UNLOCK(map_mtx);
	return 0;
}
