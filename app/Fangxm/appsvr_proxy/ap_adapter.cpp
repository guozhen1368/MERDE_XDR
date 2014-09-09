#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hlyt_hdr.h"
#include "ap_adapter.h"

static void *recv_adap = NULL;
static void *send_adap = NULL;
static void *recv_thread_handle = NULL;

static void *start_adapter(unsigned long cfghd, const char *sec, 
		bool is_server)
{
	void *adap = NULL;

	if (is_server == true)
		adap = adapter_register_ss(cfghd, (char *)sec, ap_is_running);
	else
		adap = adapter_register_if6cli(cfghd, (char *)sec);
	if (adap == NULL) {
		LOGERROR("Register adapter failed.");
		return NULL;
	}
	adapter_open(adap);
	LOGINFO("Register adapter at %p.", adap);
	return adap;
}

static void *recv_thread(void *arg)
{
	pkt_hdr *ph;
	int plen;
	unsigned char ptype;

	while (ap_is_running()) {
		ph = (pkt_hdr *)adapter_read(recv_adap);
		if (ph == NULL) {
			SLEEP_MS(10);
			continue;
		}

		ptype = pkthdr_get_type(ph);
		if (ptype != PKTTYPE_XDR && ptype != PKTTYPE_XDRRAWDATA) {
			LOGDEBUG("Incorrect packet type: 0x%02X", ptype);
			free(ph);
			continue;
		}

		plen = pkthdr_get_plen(ph);
		printf("plen:%d ptype:0x%02X\n", plen, ptype);
		adapter_write(send_adap, ph, plen);
		free(ph);
	}
	return NULL;
}

int ap_adapter_start(char *cfgfile)
{
	unsigned long cfghd;
	int ret = -1;

	if ((cfghd = CfgInitialize(cfgfile)) == 0ul)
		goto out;
	if ((recv_adap = start_adapter(cfghd, "Adapter.Recv", true)) == NULL)
		goto out;
	if ((send_adap = start_adapter(cfghd, "Adapter.Send", false)) == NULL)
		goto out;
	if ((recv_thread_handle = thread_open(recv_thread, NULL)) == NULL)
		goto out;
	ret = 0;

out:
	if (cfghd != 0ul)
		CfgInvalidate(cfghd);
	ap_adapter_stop();
	return ret;
}

int ap_adapter_stop(void)
{
	if (recv_thread_handle) {
		thread_close(recv_thread_handle);
		recv_thread_handle = NULL;
	}

	if (recv_adap) {
		adapter_close(recv_adap);
		recv_adap = NULL;
	}

	if (send_adap) {
		adapter_close(send_adap);
		send_adap = NULL;
	}

	return 0;
}
