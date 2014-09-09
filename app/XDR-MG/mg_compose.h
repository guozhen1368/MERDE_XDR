#ifndef __XDR_MG_COMPOSE_H__
#define __XDR_MG_COMPOSE_H__

#include <boost/noncopyable.hpp>

#include <timermgr.h>
#include <pkt.h>
#include <queue>

#include "mg_parse.h"
#include "mg_userdata.h"
#include "redis_help.h"

#include <mutex.h>



class XdrCompose : boost::noncopyable
{
public:
	XdrCompose(int id, unsigned long hd, RedisHelp *redis);
	~XdrCompose();

	bool init();
	void deal_xdr(pkt_hdr *ph);
	void runTimer();
	
private:
	bool set_procedure(uint8_t proc);
	int match_procedure();
	int match_begin(uint8_t proc);

	void dealTimer();

	int uploadXdr(DequeData *dequeData);

	

	int _id;
	UserData _userdata;
	DataParse _parse;
	MergeXdr _mergeXdr;
	DequeData *_dequeData;
	void* _pTimerList;
	RedisHelp *_redis;

public:
	void* _mutex;
	std::queue<DequeData *> _queReady;
	

};















#endif //__XDR_MG_COMPOSE_H__