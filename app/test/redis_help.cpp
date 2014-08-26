#include "redis_help.h"

#include <stdio.h>
#include "ae.h"




static int redis_time_cb(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
	RedisHelp *r = (RedisHelp *)clientData;
	if ( r && r->init())
	{
		r->cb_("redis_time_cb : redis connect");
		return AE_NOMORE;
	}

	return AE_NONE;
}

static void connectCallback(const redisAsyncContext *c, int status) {
	RedisHelp *r = (RedisHelp*)c->data;
	if (NULL == r)
	{
		r->cb_("RedisHelp: connectCallback RedisHelp is null");
		return;
	}

	if (status != REDIS_OK) {
		//r->setConnTimer();
		r->cb_("RedisHelp: connectCallback error %s", c->errstr);
		return;
	}

	r->setConnected(true);
	r->cb_("RedisHelp: connectCallback Connected...");
}

static void disconnectCallback(const redisAsyncContext *c, int status) {
	RedisHelp *r = (RedisHelp*)c->data;
	if (NULL == r)
	{
		r->cb_("RedisHelp: disconnectCallback RedisHelp is null");
		return;
	}
	
	r->setConnected(false);
	/*if ( !r->init())
	{
	r->setConnTimer();
	}*/

	if (status != REDIS_OK) {
		r->cb_("RedisHelp: disconnectCallback error %s", c->errstr);
		return;
	}

	r->cb_("RedisHelp: disconnectCallback Disconnected...");
}


RedisHelp::RedisHelp(char *ip, int port, long reconnSecond)
	: context_(NULL),
	  port_(port),
	  ip_(ip),
	  connected_(false),
	  reconnSecond_(reconnSecond),
	  cb_(printf)
{
	loop_ = aeCreateEventLoop(64);
	if (loop_ == NULL) {
		cb_("RedisHelp : Create loop_ fail.");
	}

	int ret = pthread_mutex_init(&mutex_, NULL);
	if (0 != ret){
		cb_("pthread_mutex_init failed error [%d]\n", ret);
	}
}

RedisHelp::~RedisHelp()
{
	if (loop_) {
		aeDeleteEventLoop(loop_);
		loop_ = NULL;
	}

	if (context_ && connected_)
	{
		redisAsyncFree(context_);
		context_ = NULL;
	}

	pthread_mutex_destroy(&mutex_);
}

bool RedisHelp::init()
{
	context_ = redisAsyncConnect(ip_.c_str(), port_);      //("127.0.0.1", 6379);
	if (context_->err) {
		/* Let *c leak for now... */
		cb_("RedisHelp: context_ error %s", context_->errstr);
		return false;
	}

	context_->data = this;

	redisAeAttach(loop_, context_);
	redisAsyncSetConnectCallback(context_,connectCallback);
	redisAsyncSetDisconnectCallback(context_,disconnectCallback);
	return true;
}

void RedisHelp::start()
{
	aeMain(loop_);
}

void RedisHelp::stop()
{
	aeStop(loop_);
}

void RedisHelp::setConnTimer()
{
	aeCreateTimeEvent(loop_, reconnSecond_*1000, redis_time_cb, (void*)this, NULL);
};

int RedisHelp::AsyncCommand(redisCallbackFn *fn, void *privdata, const char *format, ...) 
{
	va_list ap;
	int status;
	va_start(ap,format);
	lock();
	status = redisvAsyncCommand(context_, fn, privdata, format, ap);
	unlock();
	va_end(ap);
	return status;
}

int RedisHelp::AsyncCommandGet(redisCallbackFn *fn, void *privdata, const char *key, size_t keylen)
{
	int ret = -1;

	lock();
	if (connected_)
	{
		ret = redisAsyncCommand(context_, fn, privdata, "GET %b", key, keylen);
	}
	unlock();

	return ret;
}

int RedisHelp::AsyncCommandSet(const char *key, size_t keylen, const char *value, size_t valueLen)
{
	int ret = -1;

	lock();
	if (connected_)
	{
		ret = redisAsyncCommand(context_, NULL, NULL, "SET %b %b", key, keylen, value, valueLen);
	}
	unlock();

	return ret;
}

int RedisHelp::AsyncIncrGet(redisCallbackFn *fn, void *privdata, const char *key, size_t keylen)
{
	int ret = -1;

	lock();
	if (connected_)
	{
		ret = redisAsyncCommand(context_, fn, privdata, "INCRBY %b 100", key, keylen);
		//ret = redisAsyncCommand(context_, fn, privdata, "INCR %b", key, keylen);
	}
	unlock();

	return ret;
	
}
