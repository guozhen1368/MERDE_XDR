#ifndef __XDR_MG_USERDATA_H__
#define __XDR_MG_USERDATA_H__

#include <stdint.h>
#include <map>
//#include <deque>

typedef  uint64_t IMSI;

struct xdr_data_t
{
	uint64_t timstamp;
	void *data;
};

struct double_link_node_t
{
	xdr_data_t xdr_data;
	struct double_link_node_t *prev;
	struct double_link_node_t *next;
};

class DequeData
{
public:
	DequeData(const uint64_t &timstamp, void *data);
	~DequeData();

	void insert(const uint64_t &timstamp, void *data);
	void reset();


	double_link_node_t *begin(){return _begin;}
	double_link_node_t *end(){return _end;}
	void del_preNode(double_link_node_t *node);

	void setProc(uint8_t proc){_xdrProcdu = proc;};
	uint8_t getProc(){return _xdrProcdu;};

	void setProcEnd(double_link_node_t * e){_procEnd = e;};
	double_link_node_t* getProcEnd(){return _procEnd;};

	double_link_node_t* getCruNode(){return _curNode;}

	void setMmeNode(double_link_node_t * e){_mmeNode = e;}
	double_link_node_t* getMmeNode(){return _mmeNode;}

	void setTimer(void *timer){_timer = timer;}
	void* getTimer(){return _timer;};

private:
	double_link_node_t* find_node(const uint64_t &timstamp);
	double_link_node_t* create_node(const uint64_t &timstamp, void *data);

	double_link_node_t *_begin;
	double_link_node_t *_end;
	//流程消息结尾
	double_link_node_t *_procEnd;
	//新添加的node
	double_link_node_t *_curNode;

	double_link_node_t *_mmeNode;

	uint8_t _xdrProcdu;
	void* _timer;
};

class UserData
{
public:
	UserData();
	~UserData();

	DequeData *insert(const IMSI &imsi, const uint64_t &timstamp, void *data);
	void delImsi(const IMSI &imsi);

private:
	typedef std::map<IMSI, DequeData*> MAP_IMSI;
	MAP_IMSI _mapImsi;

};








#endif //__XDR_MG_USERDATA_H__