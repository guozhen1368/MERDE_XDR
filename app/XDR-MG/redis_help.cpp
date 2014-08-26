#include "redis_help.h"

#include <os.h>
#include <aplog.h>
#include <timermgr.h>

#include "redis/my_ae.h"

static void redis_time_cb(unsigned int s, unsigned int ns,
	void *data, void *arg)
{
	RedisHelp *r = (RedisHelp *)data;
	if (r)
	{
		if ( !r->init() &&  r->getTl())
		{
			TIMERLIST_ADD_TIMER_BY_OFFSET(r->getTl(), 10, 0, (void *)r);
		}
		LOG("redis_time_cb : redis connect");
	}
}

static void connectCallback(const redisAsyncContext *c, int status) {
	RedisHelp *r = (RedisHelp*)c->data;
	if (NULL == r)
	{
		LOGERROR("RedisHelp: connectCallback RedisHelp is null");
		return;
	}

	if (status != REDIS_OK) {
		TIMERLIST_ADD_TIMER_BY_OFFSET(r->getTl(), 10, 0, (void *)r);
		LOGERROR("RedisHelp: connectCallback error %s", c->errstr);
		return;
	}

	r->setConnected(true);
	LOGINFO("RedisHelp: connectCallback Connected...");
}

static void disconnectCallback(const redisAsyncContext *c, int status) {
	RedisHelp *r = (RedisHelp*)c->data;
	if (NULL == r)
	{
		LOGERROR("RedisHelp: disconnectCallback RedisHelp is null");
		return;
	}
	
	r->setConnected(false);
	if ( !r->init() &&  r->getTl())
	{
		TIMERLIST_ADD_TIMER_BY_OFFSET(r->getTl(), 10, 0, (void *)r);
	}

	if (status != REDIS_OK) {
		LOGERROR("RedisHelp: disconnectCallback error %s", c->errstr);
		return;
	}

	LOGINFO("RedisHelp: disconnectCallback Disconnected...");
}


RedisHelp::RedisHelp(char *ip, int port)
	: context_(NULL),
	  port_(port),
	  ip_(ip),
	  connected_(false),
	  mutex_(mutex_open(NULL))
{
	tl_ = TIMERLIST_CREATE(TIMERMGR_SOURCE_LOCAL, redis_time_cb,
		NULL, "redis");
	if (tl_ == NULL) {
		LOGERROR("RedisHelp : Create timerlist fail.");
	}
}

RedisHelp::~RedisHelp()
{
	if (tl_) {
		TIMERLIST_RELEASE(tl_);
		tl_ = NULL;
	}

	if (context_ && connected_)
	{
		redisAsyncFree(context_);
		context_ = NULL;
	}

	if (mutex_)
	{
		mutex_close(mutex_);
		mutex_ = NULL;
	}
}

bool RedisHelp::init()
{
	context_ = redisAsyncConnect(ip_.c_str(), port_);      //("127.0.0.1", 6379);
	if (context_->err) {
		/* Let *c leak for now... */
		LOGERROR("RedisHelp: context_ error %s", context_->errstr);
		return false;
	}

	context_->data = this;

	redisAeAttach(&loop_, context_);
	redisAsyncSetConnectCallback(context_,connectCallback);
	redisAsyncSetDisconnectCallback(context_,disconnectCallback);
	return true;
}

void RedisHelp::start()
{
	loop_.loop();
}

void RedisHelp::stop()
{
	if (tl_) {
		TIMERLIST_RELEASE(tl_);
		tl_ = NULL;
	}

	loop_.quit();
}

int RedisHelp::AsyncCommandGet(redisCallbackFn *fn, void *privdata, const char *key, size_t keylen)
{
	int ret = -1;

	mutex_lock(mutex_);
	if (connected_)
	{
		ret = redisAsyncCommand(context_, fn, privdata, "GET %b", key, keylen);
	}
	mutex_unlock(mutex_);

	return ret;
}

int RedisHelp::AsyncCommandSet(const char *key, size_t keylen, const char *value, size_t valueLen)
{
	int ret = -1;

	mutex_lock(mutex_);
	if (connected_)
	{
		ret = redisAsyncCommand(context_, NULL, NULL, "SET %b %b", key, keylen, value, valueLen);
	}
	mutex_unlock(mutex_);

	return ret;
}

int RedisHelp::AsyncIncrGet(redisCallbackFn *fn, void *privdata, const char *key, size_t keylen)
{
	int ret = -1;

	mutex_lock(mutex_);
	if (connected_)
	{
		ret = redisAsyncCommand(context_, fn, privdata, "INCRBY %b 100", key, keylen);
		//ret = redisAsyncCommand(context_, fn, privdata, "INCR %b", key, keylen);
	}
	mutex_unlock(mutex_);

	return ret;
	
}
