#ifndef __MY_REDIS_HELP_H__
#define __MY_REDIS_HELP_H__

#include <boost/noncopyable.hpp>

#include <mutex.h>

#include <hiredis.h>
#include <async.h>
#include <string>
#include "redis/my_loop.h"

class RedisHelp : boost::noncopyable
{
public:
	RedisHelp( char *ip, int port);
	~RedisHelp();

	bool init();
	void start();
	void stop();
	void *getTl(){return tl_;}

	bool getConnected(){return connected_;}
	void setConnected(bool conn){mutex_lock(mutex_); connected_ = conn; mutex_unlock(mutex_);}

	int AsyncCommandGet(redisCallbackFn *fn, void *privdata, const char *key, size_t keylen);
	int AsyncCommandSet(const char *key, size_t keylen, const char *value, size_t valueLen);
	int AsyncIncrGet(redisCallbackFn *fn, void *privdata, const char *key, size_t keylen);
	
private:
	redisAsyncContext *context_;
	int port_;
	std::string ip_;
	void *tl_;
	EventLoop loop_;
	bool connected_;
	void* mutex_;
};


#endif //__MY_REDIS_HELP_H__