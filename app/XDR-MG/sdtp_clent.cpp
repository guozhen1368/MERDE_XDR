#include "sdtp_client.h"

#include <adapter_if6cli.h>
#include <adapter.h>
#include <apfrm.h>
#include <aplog.h>


SdtpClent::SdtpClent(unsigned long cfghd)
	:adapter_(NULL),
	cfghd_(cfghd)
{

}

SdtpClent::~SdtpClent()
{
	if (adapter_)
	{
		adapter_close(adapter_);
	}
}

bool SdtpClent::init()
{
	adapter_ = adapter_register_if6cli(cfghd_, "section");
	if (NULL == adapter_)
	{
		LOGERROR("SdtpClent : init adapter is null");
		return false;
	}

	adapter_open(adapter_);
	return true;
}

int SdtpClent::sendData(unsigned char *buffer, int lenth)
{
	/*int len = 0;
	int totalLen = lenth;
	unsigned char *buf = buffer;

	pkthdr_set_plen((pkt_hdr*)buffer, lenth);*/

	if (adapter_ && buffer)
	{
		adapter_write(adapter_, buffer, lenth);
	}

	//++(adap_out.pkts);
	//adap_out.bytes += totalLen;

	/*while (totalLen > 0)
	{
	len = adapter_write(adapter_, buf, totalLen);
	if ( len < 0)
	{
	SLEEP_MS(5);
	continue;
	}

	totalLen -= len;
	buf = buf + len;
	}*/

	return 0;
}