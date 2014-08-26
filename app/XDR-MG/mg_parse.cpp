#include "mg_parse.h"
#include "msginfo.h"

#include <string.h>
#include <endian.h>
#include <sys/time.h>

#include <aplog.h>
#include "adapter.h"
#include "os.h"

struct adap_info {
	/* Adapter interface */
	void *adap;

	/* for adapter */
	bool  is_server;
	bool  is_focal;
	const char *cfg_section;

	unsigned long long pkts;
	unsigned long long bytes;
};

extern void *sdtp_client;
extern adap_info adap_out;

DataParse::DataParse()
{

}

DataParse::~DataParse()
{

}

void DataParse::parse(void *data)
{
	pkt_hdr *ph = (pkt_hdr *)data;
	if (NULL == ph)
	{
		LOGERROR("DataParse : parse data is null");
		return;
	}

	_user = (struct merge_user_t *)pkthdr_get_data(ph);;
}

uint16_t DataParse::getLenth()
{
	return be16toh(_user->lenth);
}

uint16_t DataParse::getCity()
{
	return _user->city;
}

uint8_t  DataParse::getRat()
{
	return _user->rat;
}

uint64_t DataParse::getIMSI()
{
	return _user->imsi;
}

uint64_t DataParse::getImei()
{
	return _user->imei;
}

unsigned char* DataParse::getMsisdn()
{
	return (unsigned char*)_user->msisdn;
}


uint8_t DataParse::getInterface()
{
	return _user->sig.interfac;
}

uint8_t DataParse::getProcedure()
{
	return _user->sig.procedure_type;
}

uint64_t DataParse::getStartTime()
{
	return _user->sig.procedure_start;
}

uint64_t DataParse::getEndTime()
{
	return _user->sig.procedure_end;
}

uint64_t DataParse::getStartLng()
{
	return _user->sig.start_loc_lng;
}

uint64_t DataParse::getStartLat()
{
	return _user->sig.start_loc_lat;
}

uint64_t DataParse::getEndLng()
{
	return _user->sig.end_loc_lng;
}

uint64_t DataParse::getEndLat()
{
	return _user->sig.end_loc_lat;
}

uint8_t DataParse::getStatus()
{
	return _user->sig.procedure_status;
}

uint8_t DataParse::getCause()
{
	return _user->sig.cause;
}

xdr_merge_singal_t *DataParse::getSignalData()
{
	return &(_user->sig);
}

unsigned char *DataParse::getPrivData()
{
	if (getLenth() > sizeof(merge_user_t))
	{
		return (((unsigned char*)_user) + sizeof(merge_user_t));
	}

	return NULL;
}




//////////////////////////////////////////////////


MergeXdr::MergeXdr()
	:_bufLen(0)
{
	memset(_buffer, 0xFF, BUF_LENTH);
	_mergeHead = (struct xdr_merge_head_t*)_buffer;
	_mergeSigHead = (struct xdr_merge_sig_head_t *)(_buffer + sizeof(xdr_merge_head_t));
}

MergeXdr::~MergeXdr()
{

}

void MergeXdr::reset()
{
	_xdrNumber = 0;
	_tmpXdrNumber = NULL;
	_uu = NULL;
	_s1 = NULL;
	_x2 = NULL;

	_mergeSig = NULL;

	_bufLen = sizeof(pkt_hdr) + sizeof(xdr_merge_head_t);
	memset(_buffer, 0xFF, BUF_LENTH);
}

void MergeXdr::upload(DequeData *dequeData)
{
	DequeData *de = dequeData;
	uint8_t proc = 0;

	if ( NULL == de)
	{
		LOGERROR("MergeXdr : upload de is null");
		return;
	}
	double_link_node_t *mme_node = de->getMmeNode();
	double_link_node_t *begin_node = de->begin();
	double_link_node_t *end_node = de->getProcEnd();
	if ( NULL==mme_node || NULL==begin_node || NULL==end_node)
	{
		LOGERROR("MergeXdr : upload key node is null");
		return;
	}
	proc = de->getProc();

	//处理mme
	{
		_dataParse.parse(mme_node->xdr_data.data);
		fill_merge_head(mme);
		fill_merge_sig_head(mme, proc);
	}
	
	//处理first node
	{
		_dataParse.parse(begin_node->xdr_data.data);
		fill_merge_sig_head(first);
		fill_merge_sig(first);
		_xdrNumber++;
	}
	
	//处理 node
	begin_node = begin_node->next;
	while (begin_node != end_node->next && _xdrNumber < 254)
	{
		_dataParse.parse(begin_node->xdr_data.data);
		fill_merge_sig_head(middle);
		fill_merge_sig(middle);

		_xdrNumber++;
		begin_node = begin_node->next;
	}

	//处理last node
	{
		fill_merge_sig_head(last);
		fill_merge_head(last);
	}
	
	sendData();
}

void MergeXdr::fill_merge_head(enum satus stu)
{
	if (stu == mme)
	{
		_mergeHead->imsi = _dataParse.getIMSI();
		_mergeHead->imei = _dataParse.getImei();
		unsigned char *msisdn = _dataParse.getMsisdn();
		memcpy(_mergeHead->msisdn, msisdn, 16);
		_mergeHead->city = _dataParse.getCity();
		_mergeHead->rat = _dataParse.getRat();

		//1：合成信令XDR
		_mergeHead->xdr_type = 1;
	}
	else if (stu == last)
	{
		_mergeHead->length = htole16(_bufLen);
		memset(_mergeHead->xdr_ID, 0, sizeof _mergeHead->xdr_ID);
	}
}

void MergeXdr::fill_merge_sig_head(enum satus stu, uint8_t proc)
{
	if (stu == mme)
	{
		_mergeSigHead->type = proc;

		struct s1_mme_priv_t *s1 = (struct s1_mme_priv_t *)_dataParse.getPrivData();
		if (NULL == s1)
		{
			LOGERROR("MergeXdr : fill_merge_sig_head s1 is null");
			return;
		}

		_mergeSigHead->keyword = s1->keyword;
		_mergeSigHead->mme_group_ID = s1->mme_group_ID;
		_mergeSigHead->mme_code = s1->mme_code;
		_mergeSigHead->user_IPv4 = s1->user_IPv4;
		memcpy(_mergeSigHead->user_IPv6, s1->user_IPv6, 16);
		_mergeSigHead->tac = s1->tac;
		_mergeSigHead->new_tac = s1->new_tac;

		_mergeSigHead->procedure_status = _dataParse.getStatus();


		int len = _dataParse.getLenth() - sizeof(merge_user_t) - sizeof(s1_mme_priv_t) + 1;
		memcpy(_mergeSigHead->data, s1->bear, len); 

		_bufLen += sizeof(xdr_merge_sig_head_t) + len -1;
		_tmpXdrNumber = _buffer + _bufLen;
		_bufLen++;
	}

	if (stu == first)
	{
		_mergeSigHead->start_time = _dataParse.getStartTime();
		_mergeSigHead->start_latitude = _dataParse.getStartLat();
		_mergeSigHead->start_longitude = _dataParse.getStartLng();
	}

	if (stu == middle)
	{
		if (1 == _mergeSigHead->procedure_status 
			&& 0xFF == _mergeSigHead->failure_interface
			&& 1 == _dataParse.getStatus())
		{
			_mergeSigHead->failure_interface = _dataParse.getInterface();
			uint16_t cause = _dataParse.getCause();/////? 大端
			_mergeSigHead->failure_cause = htole16(cause);
		}

		if ( XDR_TYPE_Uu == _dataParse.getInterface()
			&& 0xFFFFFFFF == _mergeSigHead->cell_ID)
		{
			struct uu_priv_t *uu = (struct uu_priv_t *)_dataParse.getPrivData();
			if (NULL == uu)
			{
				LOGERROR("MergeXdr : fill_merge_sig_head uu is null");
				return;
			}

			_mergeSigHead->cell_ID = uu->cell_ID;
			memcpy(&_mergeSigHead->eNB_ID, uu->eNB_ID, 3);/////?
		}

		if ( XDR_TYPE_X2 == _dataParse.getInterface()
			&& 0xFFFFFFFF == _mergeSigHead->new_cell_ID)
		{
			struct x2_priv_t *x2 = (struct x2_priv_t *)_dataParse.getPrivData();
			if (NULL == x2)
			{
				LOGERROR("MergeXdr : fill_merge_sig_head x2 is null");
				return;
			}

			_mergeSigHead->cell_ID = x2->cell_ID;
			memcpy(&_mergeSigHead->eNB_ID, x2->eNB_ID, 3);/////?
			_mergeSigHead->new_cell_ID = x2->new_cell_ID;
			_mergeSigHead->new_eNB_ID = x2->new_eNB_ID;
			_mergeSigHead->new_mme_code = x2->mme_code;
			_mergeSigHead->new_mme_group_ID = x2->mme_group_ID;
		
		}
	}

	if (stu == last)
	{
		_mergeSigHead->end_time = _dataParse.getEndTime();
		_mergeSigHead->end_latitude = _dataParse.getEndLat();
		_mergeSigHead->end_longitude = _dataParse.getEndLng();
		//统计单xdr数量
		*_tmpXdrNumber = _xdrNumber;
	}
}

void MergeXdr::fill_merge_sig(enum satus stu)
{
	xdr_merge_singal_t *sig = _dataParse.getSignalData();
	if (NULL == sig)
	{
		LOGERROR("MergeXdr : fill_merge_sig sig is null");
		return;
	}

	memcpy(_buffer+_bufLen, sig, sizeof(xdr_merge_singal_t));
	_bufLen += sizeof(xdr_merge_singal_t);
}

void MergeXdr::sendData()
{
	int len = 0;
	int totalLen = _bufLen;
	unsigned char *buf = _buffer;

	pkthdr_set_plen((pkt_hdr*)_buffer, _bufLen);

	if (NULL == sdtp_client)
	{
		LOGERROR("MergeXdr : sendData sdtp_client is null");
		return;
	}

	++(adap_out.pkts);
	adap_out.bytes += totalLen;

	while (totalLen > 0)
	{
		len = adapter_write(sdtp_client, buf, totalLen);
		if ( len < 0)
		{
			SLEEP_MS(5);
			continue;
		}

		totalLen -= len;
		buf = buf + len;
	}
}

	 
	 