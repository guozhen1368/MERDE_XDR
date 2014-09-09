#ifndef __GENERATE_XDR_ID_H__
#define __GENERATE_XDR_ID_H__

#include <boost/noncopyable.hpp>


// mac + pid + tm + num
#pragma pack(1)
struct XDRID{
	unsigned char    mac[6];
	unsigned short   pid;
	unsigned int     tm;
	unsigned int     num;
};
#pragma pack()

class GenerateXdrId : boost::noncopyable
{
public:
	static GenerateXdrId *instance();

	void getUniqueXdrid(XDRID & xdrid);

private:
	GenerateXdrId();
	~GenerateXdrId(){}

	XDRID xdr_;
	static GenerateXdrId *single_xdrid_g_;
};

#endif // __GENERATE_XDR_ID_H__
