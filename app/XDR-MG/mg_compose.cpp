#include "mg_compose.h"
#include "msginfo.h"
#include "datainfo.h"

#include <endian.h>
#include <os.h>
#include <string.h>

#include <aplog.h>

struct key_info
{
	uint8_t Interface;
	uint8_t Procedure;
	//uint8_t Status;
	//uint8_t Keyword;
};

struct key_info key_begin[18][2] = {
	{{0, 0}, {0, 0}},
	{{XDR_TYPE_Uu, RRC_CONN_STP}, {0, 0}}, //Attach = 1
	{{XDR_TYPE_Uu, RRC_CONN_STP}, {0, 0}},//Sevice_Request = 2
	{{XDR_TYPE_S1_MME, s1_Paging}, {0, 0}}, //Paging = 3
	{{XDR_TYPE_Uu, RRC_CONN_STP}, {0, 0}}, //TAU = 4
	{{XDR_TYPE_Uu, RRC_CONN_STP}, {XDR_TYPE_S1_MME, s1_Detach}}, //Detach = 5
	{{XDR_TYPE_Uu, RRC_CONN_STP}, {XDR_TYPE_S1_MME, s1_PDN_connectivity}}, //PDN_connectivity = 6
	{{XDR_TYPE_S1_MME, s1_PDN_disconnection}, {XDR_TYPE_Uu, RRC_CONN_STP}}, //PDN_disconnection = 7
	{{XDR_TYPE_Uu, RRC_CONN_STP}, {0, 0}}, //EPS_resource_allocation = 8
	{{XDR_TYPE_Uu, RRC_CONN_STP}, {0, 0}}, //EPS_resource_modify = 9
	{{XDR_TYPE_Uu, RRC_CONN_STP}, {XDR_TYPE_S1_MME, s1_EPS_context_deactivation}}, //EPS_context_deactivation = 10
	{{XDR_TYPE_S1_MME, s1_EPS_context_modification}, {0, 0}}, //EPS_context_modification = 11
	{{XDR_TYPE_S1_MME, s1_EPS_context_activation}, {0, 0}}, //EPS_context_activation = 12
	{{XDR_TYPE_X2, handover}, {0, 0}}, //X2_handover = 13
	{{XDR_TYPE_S1_MME, s1_S1_out}, {XDR_TYPE_S1_MME, s1_S1_in}}, //S1_handover = 14
    {{XDR_TYPE_S1_MME, s1_UE_context_release}, {0, 0}}, //UE_Context_Release = 15
    {{XDR_TYPE_S1_MME, s1_E_RAB_release}, {XDR_TYPE_Uu, RRC_CONN_STP}}, //EPS_Release = 16
	{{XDR_TYPE_Uu, RRC_CONN_STP}, {XDR_TYPE_SGs, PAGING}}, //CSFB = 17
};

static void getRedisIncrCallback(redisAsyncContext *c, void *r, void *privdata) {
	redisReply *reply = (redisReply *)r;
	if (reply == NULL) return;
	
	XdrCompose *xdr = (XdrCompose*)privdata;
	if (NULL == xdr)
	{
		LOGERROR("XdrCompose : getRedisIncrCallback xdr is null");
		return;
	}
	xdr->setXdrId(reply->integer);
	LOGINFO("XdrCompose : getRedisIncrCallback xdrid =%llu.", reply->integer);

}

static void getRedisCallback(redisAsyncContext *c, void *r, void *privdata) {
	redisReply *reply = (redisReply *)r;
	if (reply == NULL) return;
	
	DequeData *de = (DequeData*)privdata;
	if (NULL == de)
	{
		LOGERROR("XdrCompose : getRedisCallback de is null");
		return;
	}
	//设置de中的位置信息
	//de
	LOGINFO("XdrCompose : getRedisCallback %s", reply->str);
}

// Timer callback function
static void XdrTimeoutCallback(unsigned int s, unsigned int ns,
	void *data, void *arg)
{
	XdrCompose* compose = (XdrCompose*)arg;
	DequeData* de = (DequeData*)data;

	if (compose && de)
	{
		mutex_lock(compose->_mutex);
		compose->_queReady.push(de);
		mutex_unlock(compose->_mutex);
	}
}

XdrCompose::XdrCompose(int id, unsigned long hd, RedisHelp *redis)
	:_id(id),
	_dequeData(NULL),
	_redis(redis),
	curXdrID_(0),
	maxXdrID_(0)
{

}

XdrCompose::~XdrCompose()
{
	if (_pTimerList)
	{
		timer_mgr_release_timerlist(NULL, _pTimerList);
	}

	if (_mutex)
	{
		mutex_close(_mutex);
	}
}

bool XdrCompose::init()
{
	_pTimerList = timer_mgr_create_timerlist(NULL, TIMERMGR_SOURCE_LOCAL,
		XdrTimeoutCallback, this, "xdrcompose");
	if (_pTimerList == NULL) {
		LOGERROR("XdrCompose: Failed to create timerlist.");
		return false;
	}

	_mutex = mutex_open(NULL);
	if (NULL == _mutex)
	{
		LOGERROR("XdrCompose: Failed to create _mutex.");
		return false;
	}

	getXdrId();

	return true;
}

uint64_t XdrCompose::getXdrId()
{
	static const char *xdrid = "SIG_XDR_ID";
	if (_redis && maxXdrID_-curXdrID_ < 10)
	{
		_redis->AsyncIncrGet(getRedisIncrCallback, this, xdrid, strlen(xdrid));
	}

	return curXdrID_==0?curXdrID_:curXdrID_++;
}

void XdrCompose::setXdrId(uint64_t xdrid)
{
	maxXdrID_ = xdrid;
	if (0 == curXdrID_)
	{
		curXdrID_ = maxXdrID_ - 99;
	}
}

void XdrCompose::deal_xdr(pkt_hdr *ph)
{
	if (NULL == ph)
	{
		LOGERROR("deal_xdr param data is null");
		return;
	}

	_parse.parse(ph);
	uint64_t imsi = _parse.getIMSI();
	uint64_t startTime = be64toh(_parse.getStartTime());

	//将data插入以imsi为key的链表
	_dequeData = _userdata.insert(imsi, startTime, ph);

	_redis->AsyncCommandGet(getRedisCallback, _dequeData, (char*)&imsi, sizeof(uint64_t));

	//匹配流程类型、上报
	match_procedure();

	//串行处理timer返回的事件
	dealTimer();
}

bool XdrCompose::set_procedure(uint8_t proc)
{
	//匹配消息尾开头
	match_begin(proc);

	//上报此流程消息头结点之前的数据
	if (0 != _dequeData->getProc())
	{
		void *timer = _dequeData->getTimer();
		if (timer) {
			TIMERLIST_DEL_TIMER(_pTimerList, timer);
		}
		_dequeData->setTimer(NULL);

		uploadXdr(_dequeData);
	}

	_dequeData->setProc(proc);
	_dequeData->setMmeNode(_dequeData->getCruNode());

	//设置超时
	void *timer = TIMERLIST_ADD_TIMER_BY_OFFSET(_pTimerList, 2, 0, _dequeData);
	_dequeData->setTimer(timer);

	return true;
}

int XdrCompose::match_procedure()
{
	//当前流程匹配
	
	if (XDR_TYPE_S1_MME != _parse.getInterface())
	{
		return 0;
	}


	switch (_parse.getProcedure())
	{
	case s1_Attach:
		set_procedure(Attach);
		break;
	case s1_Sevice_Request:
		set_procedure(Sevice_Request);
		break;
	case s1_Paging:
		set_procedure(Paging);
		break;
	case s1_TAU:
		set_procedure(TAU);
		break;
	case s1_Detach:
		set_procedure(Detach);
		break;
	case s1_PDN_connectivity:
		set_procedure(PDN_connectivity);
		break;
	case s1_PDN_disconnection:
		set_procedure(PDN_disconnection);
		break;
	case s1_EPS_resource_allocation:
		set_procedure(EPS_resource_allocation);
		break;
	case s1_EPS_resource_modify:
		set_procedure(EPS_resource_modify);
		break;
	case s1_EPS_context_deactivation:
		set_procedure(EPS_context_deactivation);
		break;
	case s1_EPS_context_modification:
		set_procedure(EPS_context_modification);
		break;
	case s1_EPS_context_activation:
		set_procedure(EPS_context_activation);
		break;
	case s1_X2_change:
		set_procedure(X2_handover);
		break;
	case s1_S1_out:
	case s1_S1_in:
		set_procedure(S1_handover);
		break;
	case s1_UE_context_release:
		set_procedure(UE_Context_Release);
		break;
	case s1_E_RAB_release:
		set_procedure(EPS_Release);
		break;
	case s1_Extended_Service_Request:
		{
			struct s1_mme_priv_t *s1 = (struct s1_mme_priv_t *)_parse.getPrivData();
			if (NULL == s1)
			{
				LOGERROR("MergeXdr : fill_merge_sig_head s1 is null");
				return -1;
			}

			//0 ~ 3 csbf 
			if (s1->keyword >= 0 && s1->keyword <= 3)
			{
				set_procedure(CSFB);
			}
		}
		
		break;
	}

	return 0;
}


int XdrCompose::match_begin(uint8_t proc)
{
	double_link_node_t *begin_node = _dequeData->getMmeNode();
	double_link_node_t *end_node = _dequeData->getCruNode();//second mme
	if (NULL == begin_node)
	{
		_dequeData->setProcEnd(NULL);
		return 0;
	}

	//second mme prev node
	end_node = end_node->prev;
	_dequeData->setProcEnd(end_node);

	//取 begin 和 end之间(两个mme之间);
	while (begin_node != end_node)
	{
		_parse.parse(end_node->xdr_data.data);
		/////////////
		for (uint8_t i=0; i<2; i++)
		{
			//match second mme procedure start node
			if (key_begin[proc][i].Interface == _parse.getInterface() 
				&& key_begin[proc][i].Procedure == _parse.getProcedure())
			{
				//set current mme procedure end node
				_dequeData->setProcEnd(end_node->prev);
				goto end;
			}
		}
		
		end_node = end_node->prev;
	}

end:
	LOGINFO("XdrCompose : match procedure = %d", _dequeData->getProc());

	return 0;
}

int XdrCompose::uploadXdr(DequeData *dequeData)
{
	DequeData* de = dequeData;

	//compose data upload
	if (NULL == de)
	{
		LOGERROR("XdrCompose : uploadXdr de is null");
		return -1;
	}

	//合成、上报xdr
	{
		_mergeXdr.reset();
		_mergeXdr.upload(de);
	}

	//上报结束，清除数据
	de->reset();

	return  0;
}


void XdrCompose::dealTimer()
{
	DequeData* de = NULL;

	while (!_queReady.empty())
	{
		mutex_lock(_mutex);
		de = _queReady.front();
		_queReady.pop();
		mutex_unlock(_mutex);

		//timer == null uploadXdr exec in set_procedure()
		if (NULL == de || NULL == de->getTimer())
		{
			continue;
		}
		
		//timeout set current node eq procend node
		de->setProcEnd(de->getCruNode());
		uploadXdr(de);
		
	}
}