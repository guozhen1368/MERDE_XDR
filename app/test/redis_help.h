#ifndef __MY_REDIS_HELP_H__
#define __MY_REDIS_HELP_H__

#include <pthread.h>

#include <hiredis.h>
#include <async.h>
#include <string>
#include <queue>

extern "C"{
#include "ae/ae.h"
}

typedef int (*logCallBack)(const char *format, ...);

//struct cmd_t {
//	redisCallbackFn cbk;
//	void *param;
//
//	const char *fmt;
//	va_list ap;
//};

class RedisHelp
{
public:
	RedisHelp( char *ip, int port, long reconnSecond = 10);
	~RedisHelp();

	bool init();
	void start();
	void stop();
	void setConnCallback(logCallBack cb){cb_ = cb;}
	const char * getRedisError(){return context_->c.errstr;}

	bool getConnected(){return connected_;}
	void setConnected(bool conn){lock(); connected_ = conn; unlock();}
	void setConnTimer();

	//int AsyncCommandGet(redisCallbackFn *fn, void *privdata, const char *fmt, ...);
	int AsyncCommand(redisCallbackFn *fn, void *privdata, const char *format, ...); 

	int AsyncCommandGet(redisCallbackFn *fn, void *privdata, const char *key, size_t keylen);
	int AsyncCommandSet(const char *key, size_t keylen, const char *value, size_t valueLen);
	int AsyncIncrGet(redisCallbackFn *fn, void *privdata, const char *key, size_t keylen);
	
	logCallBack cb_;
private:
	static int timerBack(struct aeEventLoop *eventLoop, long long id, void *clientData);
	
	void lock(){};//pthread_mutex_lock(&mutex_);}
	void unlock(){};//pthread_mutex_unlock(&mutex_);}

	redisAsyncContext *context_;
	int port_;
	std::string ip_;
	aeEventLoop *loop_;
	bool connected_;
	pthread_mutex_t mutex_;
	long reconnSecond_;

	//std::queue<cmd_t> cmdQueue_;
};


#endif //__MY_REDIS_HELP_H__