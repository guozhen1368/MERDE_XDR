#ifndef __SDTP_CLENT_H__
#define __SDTP_CLENT_H__


#include <boost/noncopyable.hpp>

class SdtpClent : boost::noncopyable
{
public:
	explicit SdtpClent(unsigned long cfghd);
	~SdtpClent();

	bool init();
	int sendData(unsigned char *buf, int len);

private:
	void *adapter_;
	unsigned long cfghd_;

};


#endif //__SDTP_CLENT_H__