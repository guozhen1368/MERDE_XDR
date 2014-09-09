#include "xdr_handler.h"
#include "xdr_proc.h"
#include "xdr_proc_u.h"
#include "xdr_proc_s.h"
#include "xdr_pkt.h"
#include "getcfg_v2.h"

static int handle_mode;

int xdrproc_start(int mode)
{
	int fill_timeout = 30;

	handle_mode = mode;

	getcfg_v2(CFGFILE, "global", "fill_timeout", &fill_timeout,
			GETCFG_INT32);
	if (fill_timeout <= 0)
		fill_timeout = 30;

	if (handle_mode == XDR_MODE_U)
		return xdrproc_u_start(fill_timeout);
	else if (handle_mode == XDR_MODE_S)
		return xdrproc_s_start(fill_timeout);
	return -1;
}

int xdrproc_add(char *xdr, int bytes)
{
	if (handle_mode == XDR_MODE_U)
		return xdrproc_u_add(xdr, bytes);
	else if (handle_mode == XDR_MODE_S)
		return xdrproc_s_add(xdr, bytes);
	return -1;
}
