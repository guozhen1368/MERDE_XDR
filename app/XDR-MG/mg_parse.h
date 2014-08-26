#ifndef __XDR_MG_PARSE_H__
#define __XDR_MG_PARSE_H__

#include <boost/noncopyable.hpp>

#include <pkt.h>

#include "datainfo.h"
#include "mg_userdata.h"
#include "redis_help.h"

class DataParse : boost::noncopyable
{
public:
	DataParse();
	~DataParse();

	void parse(void *data);

	/////get param
	uint16_t getLenth();
	uint16_t getCity();
	uint8_t  getRat();
	uint64_t getIMSI();
	uint64_t getImei();
	unsigned char* getMsisdn();

	uint8_t getInterface();
	uint8_t getProcedure();
	uint64_t getStartTime();
	uint64_t getEndTime();
	uint64_t getStartLng();
	uint64_t getStartLat();
	uint64_t getEndLng();
	uint64_t getEndLat();
	uint8_t getStatus();
	uint8_t getCause();

	xdr_merge_singal_t *getSignalData();
	unsigned char *getPrivData();

private:

	struct merge_user_t *_user;
};

class MergeXdr : boost::noncopyable
{
public:
	enum satus
	{
		mme,
		first,
		middle,
		last
	};

	MergeXdr();
	~MergeXdr();

	void reset();
	void upload(DequeData *dequeData);

private:
	void fill_merge_head(enum satus stu);
	void fill_merge_sig_head( enum satus stu, uint8_t proc = 0);
	void fill_merge_sig(enum satus stu);
	void sendData();

	struct xdr_merge_head_t *_mergeHead;
	struct xdr_merge_sig_head_t *_mergeSigHead;
	struct xdr_merge_singal_t *_mergeSig;

	DataParse _dataParse;
	struct uu_priv_t *_uu;
	struct s1_mme_priv_t *_s1;
	struct x2_priv_t *_x2;

	unsigned char *_tmpXdrNumber;
	uint8_t _xdrNumber;
	static const int BUF_LENTH = 64*1024;
	unsigned char _buffer[64*1024];
	uint16_t _bufLen;
};

#endif //__XDR_MG_PARSE_H__