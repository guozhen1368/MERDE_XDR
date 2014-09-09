/*
 * appsvr - receive XDR and save it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hlyt_hdr.h"
#include "ap_adapter.h"

static char *cfgfile = (char *)"appsvr.cfg";
static char *pidfile = (char *)"/tmp/appsvr.pid";

static int app_main(long instance, unsigned long data)
{
	if (ap_adapter_start(cfgfile) != 0) {
		LOGERROR("ap_adapter_start() failed.");
		return -1;
	}

	while (ap_is_running()) {
		sleep(3);
	}

	ap_adapter_stop();
	return 0;
}

static struct ap_framework my_app = {
	pidfile,
	app_main,
	0ul, NULL, NULL, NULL, NULL, NULL, NULL,
};

#if defined(__cplusplus)
extern "C"
{
#endif

struct ap_framework *register_ap(void)
{
	return &my_app;
}

#if defined(__cplusplus)
}
#endif
