#include "mg_parse.h"
#include "msginfo.h"

#include <string.h>
#include <endian.h>
#include <sys/time.h>

#include <aplog.h>
#include "adapter.h"
#include "os.h"
#include "calc_xdrid.h"

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

	user_ = (struct merge_user_t *)pkthdr_get_data(ph);
	lenth_ = pkthdr_get_dlen(ph);

	if (lenth_ != PKT_NTOHS(user_->lenth))
	{
		LOGERROR("DataParse::parse  data ivalid dlen=%d ulen=%d!\n", lenth_, PKT_NTOHS(user_->lenth));
	}
}

uint16_t DataParse::getLenth()
{
	return lenth_;
}

uint16_t DataParse::getCity()
{
	return user_->city;
}

uint8_t  DataParse::getRat()
{
	return user_->rat;
}

uint64_t DataParse::getIMSI()
{
	return user_->imsi;
}

uint64_t DataParse::getImei()
{
	return user_->imei;
}

unsigned char* DataParse::getMsisdn()
{
	return (unsigned char*)user_->msisdn;
}


uint8_t DataParse::getInterface()
{
	return user_->sig.interfac;
}

uint8_t DataParse::getProcedure()
{
	return user_->sig.procedure_type;
}

uint64_t DataParse::getStartTime()
{
	return user_->sig.procedure_start;
}

uint64_t DataParse::getEndTime()
{
	return user_->sig.procedure_end;
}

uint64_t DataParse::getStartLng()
{
	return user_->sig.start_loc_lng;
}

uint64_t DataParse::getStartLat()
{
	return user_->sig.start_loc_lat;
}

uint64_t DataParse::getEndLng()
{
	return user_->sig.end_loc_lng;
}

uint64_t DataParse::getEndLat()
{
	return user_->sig.end_loc_lat;
}

uint8_t DataParse::getStatus()
{
	return user_->sig.procedure_status;
}

uint8_t DataParse::getCause()
{
	return user_->sig.cause;
}

xdr_merge_singal_t *DataParse::getSignalData()
{
	return &(user_->sig);
}

unsigned char *DataParse::getPrivData() 
{
	if (getLenth() > sizeof(merge_user_t))
	{
		return (((unsigned char*)user_) + sizeof(merge_user_t));
	}

	return NULL;
}




//////////////////////////////////////////////////


MergeXdr::MergeXdr()
	:bufLen_(0)
{
	reset();
}

MergeXdr::~MergeXdr()
{

}

void MergeXdr::reset()
{
	xdrNumber_ = 0;
	tmpXdrNumber_ = NULL;
	/*uu_ = NULL;
	_s1 = NULL;
	_x2 = NULL;*/

	memset(buffer_, 0xFF, BUF_LENTH);

	mg_sig_ = (mg_sig_t *)buffer_;
	bufLen_ = sizeof(mg_sig_t); 

	mg_sig_->xdr_type = SDTP_XDR_MERGE;
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
		dataParse_.parse(mme_node->xdr_data.data);
		fill_merge_head(mme);
		fill_merge_sig_head(mme, proc);
	}
	
	//处理first node
	{
		dataParse_.parse(begin_node->xdr_data.data);
		fill_merge_sig_head(first);
		fill_merge_sig_head(middle);
		fill_merge_sig(first);
		xdrNumber_++;
	}
	
	//处理 node
	begin_node = begin_node->next;
	while (begin_node != end_node->next && xdrNumber_ < 254)
	{
		dataParse_.parse(begin_node->xdr_data.data);
		fill_merge_sig_head(middle);
		fill_merge_sig(middle);

		xdrNumber_++;
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
		mg_sig_->mergeHead.imsi = dataParse_.getIMSI();
		mg_sig_->mergeHead.imei = dataParse_.getImei();
		unsigned char *msisdn = dataParse_.getMsisdn();
		memcpy(mg_sig_->mergeHead.msisdn, msisdn, 16);
		mg_sig_->mergeHead.city = dataParse_.getCity();
		mg_sig_->mergeHead.rat = dataParse_.getRat();

		XDRID id;
		GenerateXdrId::instance()->getUniqueXdrid(id);
		memcpy(mg_sig_->mergeHead.xdr_ID, &id, sizeof id);

		//1：合成信令XDR
		mg_sig_->mergeHead.xdr_type = 1;
	}
	else if (stu == last)
	{
		mg_sig_->mergeHead.length = htole16(bufLen_);
	}
}

void MergeXdr::fill_merge_sig_head(enum satus stu, uint8_t proc)
{
	if (stu == mme)
	{
		mg_sig_->mergeSigHead.type = proc;

		struct s1_mme_priv_t *s1 = (struct s1_mme_priv_t *)dataParse_.getPrivData();
		if (NULL == s1)
		{
			LOGERROR("MergeXdr : fill_merge_sig_head s1 is null");
			return;
		}

		mg_sig_->mergeSigHead.keyword = s1->keyword;
		mg_sig_->mergeSigHead.mme_group_ID = s1->mme_group_ID;
		mg_sig_->mergeSigHead.mme_code = s1->mme_code;
		mg_sig_->mergeSigHead.user_IPv4 = s1->user_IPv4;
		memcpy(mg_sig_->mergeSigHead.user_IPv6, s1->user_IPv6, 16);
		mg_sig_->mergeSigHead.tac = s1->tac;
		mg_sig_->mergeSigHead.new_tac = s1->new_tac;

		mg_sig_->mergeSigHead.procedure_status = dataParse_.getStatus();


		int len = dataParse_.getLenth() - sizeof(merge_user_t) - sizeof(s1_mme_priv_t);
		memcpy(mg_sig_->mergeSigHead.data, s1->bear, len); 

		bufLen_ +=  len;
		tmpXdrNumber_ = &buffer_[bufLen_];
		bufLen_++;
	}

	if (stu == first)
	{
		mg_sig_->mergeSigHead.start_time = dataParse_.getStartTime();
		mg_sig_->mergeSigHead.start_latitude = dataParse_.getStartLat();
		mg_sig_->mergeSigHead.start_longitude = dataParse_.getStartLng();
	}

	if (stu == middle)
	{
		if (1 == mg_sig_->mergeSigHead.procedure_status 
			&& 0xFF == mg_sig_->mergeSigHead.failure_interface
			&& 1 == dataParse_.getStatus())
		{
			mg_sig_->mergeSigHead.failure_interface = dataParse_.getInterface();
			uint16_t cause = dataParse_.getCause();/////? 大端
			mg_sig_->mergeSigHead.failure_cause = htole16(cause);
		}

		if ( XDR_TYPE_Uu == dataParse_.getInterface()
			&& 0xFFFFFFFF == mg_sig_->mergeSigHead.cell_ID)
		{
			struct uu_priv_t *uu = (struct uu_priv_t *)dataParse_.getPrivData();
			if (NULL == uu)
			{
				LOGERROR("MergeXdr : fill_merge_sig_head uu is null");
				return;
			}

			mg_sig_->mergeSigHead.cell_ID = uu->cell_ID;
			mg_sig_->mergeSigHead.eNB_ID = uu->eNB_ID;
		}

		if ( XDR_TYPE_X2 == dataParse_.getInterface()
			&& 0xFFFFFFFF == mg_sig_->mergeSigHead.new_cell_ID)
		{
			struct x2_priv_t *x2 = (struct x2_priv_t *)dataParse_.getPrivData();
			if (NULL == x2)
			{
				LOGERROR("MergeXdr : fill_merge_sig_head x2 is null");
				return;
			}

			mg_sig_->mergeSigHead.cell_ID = x2->cell_ID;
			mg_sig_->mergeSigHead.eNB_ID = x2->eNB_ID;
			mg_sig_->mergeSigHead.new_cell_ID = x2->new_cell_ID;
			mg_sig_->mergeSigHead.new_eNB_ID = x2->new_eNB_ID;
			mg_sig_->mergeSigHead.new_mme_code = x2->mme_code;
			mg_sig_->mergeSigHead.new_mme_group_ID = x2->mme_group_ID;
		
		}
	}

	if (stu == last)
	{
		mg_sig_->mergeSigHead.end_time = dataParse_.getEndTime();
		mg_sig_->mergeSigHead.end_latitude = dataParse_.getEndLat();
		mg_sig_->mergeSigHead.end_longitude = dataParse_.getEndLng();
		//统计单xdr数量
		*tmpXdrNumber_ = xdrNumber_;
	}
}

void MergeXdr::fill_merge_sig(enum satus stu)
{
	xdr_merge_singal_t *sig = dataParse_.getSignalData();
	if (NULL == sig)
	{
		LOGERROR("MergeXdr : fill_merge_sig sig is null");
		return;
	}

	memcpy(buffer_+bufLen_, sig, sizeof(xdr_merge_singal_t));
	bufLen_ += sizeof(xdr_merge_singal_t);
}

void MergeXdr::sendData()
{
	//int len = 0;
	int totalLen = bufLen_;
	unsigned char *buf = buffer_;

	pkthdr_set_plen((pkt_hdr*)buffer_, bufLen_);
	pkthdr_set_sync((pkt_hdr*)buffer_);
	pkthdr_set_type((pkt_hdr*)buffer_, PKTTYPE_XDR);


	if (NULL == adap_out.adap)
	{
		LOGERROR("MergeXdr : sendData sdtp_client is null");
		return;
	}

	++(adap_out.pkts);
	adap_out.bytes += totalLen;

	adapter_write(adap_out.adap, buf, totalLen);

	/*while (totalLen > 0)
	{
	len = adapter_write(adap_out.adap, buf, totalLen);
	if ( len < 0)
	{
	SLEEP_MS(5);
	continue;
	}

	totalLen -= len;
	buf = buf + len;
	}*/
}

	 
	 