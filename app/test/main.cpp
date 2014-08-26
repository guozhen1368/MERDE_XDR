#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "redis_help.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/* Put event loop in the global scope, so it can be explicitly stopped */

static unsigned int count_g = 0;
static time_t s;

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply *)r;
    if (reply == NULL) return;
	/*if (reply->type == REDIS_REPLY_ERROR)
	{
	printf("argv[error]: %s\n", reply->str);
	} 
	else if(reply->type == REDIS_REPLY_STRING)
	{
	printf("argv[%s]: %s\n", (char*)privdata, reply->str);
	}
	else if (reply->type == REDIS_REPLY_INTEGER)
	{
	printf("argv[%s]: %llu\n", (char*)privdata, reply->integer);
	}
	else
	{
	printf("argv[other]: type=%d str=%s\n", reply->type, reply->str);
	}*/

	count_g++;
	if (count_g%10000 == 0)
	{
		printf("time = %lu    count = %u \n", time(NULL)-s, count_g);
	}
}

void *proc(void *ca)
{
	RedisHelp *loop = (RedisHelp*)ca;
    loop->init();
	loop->start();
	
}

int main (int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

    RedisHelp redis("127.0.0.1", 6379, 10);
	

	pthread_t ph;
	pthread_create(&ph, NULL, proc, (void*)&redis);

    sleep(2);
    
    s = time(NULL);
	printf("start time %llu\n", s);
	int count = 0;
   int ret = 0;
	while (1)
	{
		ret = redis.AsyncCommand(getCallback, (char*)"set", "some", 4, "any", 3);
		if (ret<0)
		{
			printf("some ret =%s", redis.getRedisError());
			usleep(10*1000);
		}
		ret = redis.AsyncCommandGet(getCallback, (char*)"get", "some", 4);
		if (ret<0)
		{
			printf("some ret =%s", redis.getRedisError());
			usleep(10*1000);
		}
		const char * aaa = "ssdsdf";
		ret = redis.AsyncIncrGet(getCallback, (char*)"incr", aaa, strlen(aaa));
		if (ret<0)
		{
			printf("some ret =%s", redis.getRedisError());
			usleep(10*1000);
		}

		count++;
		if (count%2000 == 0)
		{
			usleep(1000);
		}
	}
	printf("time = %lu    count = %u \n", time(NULL)-s, count_g);
	
    return 0;
}

