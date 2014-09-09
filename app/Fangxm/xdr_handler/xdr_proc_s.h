#ifndef _XDR_PROC_S_H_
#define _XDR_PROC_S_H_

#include <list>
#include <time.h>
#include "xdr_pkt.h"

#define MAP_S_SIZE_MAX	1000000

using namespace std;

struct map_s_val_t {
	int      filled;
	byte     imei[8];
	byte     msisdn[16];
	time_t   last_arrived;

	list<struct xdr_pkt_t> lst;
};

#ifdef __cplusplus
extern "C"
{
#endif

int xdrproc_s_start(int fill_timeout);

int xdrproc_s_add(char *xdr, int bytes);

#ifdef __cplusplus
}
#endif

#endif	/* _XDR_PROC_S_H_ */
