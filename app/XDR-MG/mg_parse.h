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

	struct merge_user_t *user_;
	uint16_t lenth_;
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

	struct mg_sig_t{
		pkt_hdr              ph;
		uint8_t              xdr_type;
		xdr_merge_head_t     mergeHead;
		xdr_merge_sig_head_t mergeSigHead;
	};

	mg_sig_t *mg_sig_;
	//pkt_hdr          *ph_;
	//struct xdr_merge_head_t *mergeHead_;
	//struct xdr_merge_sig_head_t *mergeSigHead_;

	DataParse dataParse_;
	/*struct uu_priv_t *uu_;
	struct s1_mme_priv_t *_s1;
	struct x2_priv_t *_x2;*/

	unsigned char *tmpXdrNumber_;
	uint8_t xdrNumber_;
	static const int BUF_LENTH = 64*1024;
	unsigned char buffer_[64*1024];
	uint16_t bufLen_;
};

#endif //__XDR_MG_PARSE_H__