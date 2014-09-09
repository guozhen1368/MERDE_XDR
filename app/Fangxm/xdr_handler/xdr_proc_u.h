#ifndef _XDR_PROC_U_H_
#define _XDR_PROC_U_H_

#include <list>
#include <time.h>
#include "xdr_pkt.h"

#define MAP_U_SIZE_MAX	1000000

using namespace std;

struct map_u_val_t {
	int      filled;
	byte     imsi[8];
	byte     imei[8];
	byte     msisdn[16];
	byte     enb_id[3];
	byte     cell_id[4];
	time_t   last_arrived;

	list<struct xdr_pkt_t> lst;
};

#ifdef __cplusplus
extern "C"
{
#endif

int xdrproc_u_start(int fill_timeout);

int xdrproc_u_add(char *xdr, int bytes);

#ifdef __cplusplus
}
#endif

#endif	/* _XDR_PROC_U_H_ */
