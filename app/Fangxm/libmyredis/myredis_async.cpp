#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include "hiredis/hiredis.h"
#include "myredis_async.h"

using namespace std;

#define LOCK(mtx)	pthread_mutex_lock(&mtx)
#define UNLOCK(mtx)	pthread_mutex_unlock(&mtx)

struct cmd_t {
	myredis_async_callback_t cbk;
	void *param;

	char *cmd;
	int   len;
};

static queue<cmd_t> cmd_queue;
static pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER;

struct myredis_async_handle_t {
	redisContext *c;
	char ip[32];
	int port;
	int exit;
	sem_t sem;
	pthread_t work_tid;
};

/* if return -1, need to re-connect redis server */
static int exec_cmd(struct myredis_async_handle_t *handle)
{
	struct cmd_t cmd;
	int ret;
	redisReply *reply = NULL;

	LOCK(queue_mtx);
	if (cmd_queue.empty()) {
		UNLOCK(queue_mtx);
		return 0;
	}
	cmd = cmd_queue.front();
	cmd_queue.pop();
	UNLOCK(queue_mtx);

	ret = redisAppendFormattedCommand(handle->c, cmd.cmd, cmd.len);
	free(cmd.cmd);

	if (ret == REDIS_ERR) {
		if (handle->c->err) {
			printf("redisAppendFormattedCommand() failed: %s\n", 
					handle->c->errstr);
			return -1;
		}
		return 0;
	}

	ret = redisGetReply(handle->c, (void **)&reply);
	if (ret == REDIS_ERR || reply == NULL)
		return 0;

	if (cmd.cbk)
		cmd.cbk(reply->str, reply->len, cmd.param);
	freeReplyObject(reply);
	return 0;
}

static void *work_thread(void *arg)
{
	struct myredis_async_handle_t *handle = 
		(struct myredis_async_handle_t *)arg;
	struct timeval tv = {5, 0};
	int print_err = 1;

conn_redis:
	if (handle->exit == 1)
		goto out;

	if (handle->c) {
		redisFree(handle->c);
		handle->c = NULL;
	}

	handle->c = redisConnectWithTimeout(handle->ip, handle->port, tv);
	if (handle->c == NULL || handle->c->err) {
		if (print_err) {
			printf("Connection error: %s\n", handle->c != NULL ?
					handle->c->errstr :
					"allocate redis context");
			print_err = 0;
		}
		sleep(5);
		goto conn_redis;
	}
	redisEnableKeepAlive(handle->c);
	redisSetTimeout(handle->c, tv);
	print_err = 1;
	printf("connect on redis-server\n");

	while (handle->exit == 0) {
		if (sem_wait(&handle->sem) == 0) {
			if (exec_cmd(handle) == -1) {
				sleep(5);
				goto conn_redis;
			}
		}
	}
out:
	return NULL;
}

struct myredis_async_handle_t *myredis_async_init(char *ip, int port)
{
	struct myredis_async_handle_t *handle;
	
	handle = (struct myredis_async_handle_t *)malloc(
			sizeof(struct myredis_async_handle_t));
	if (handle == NULL)
		goto err;
	handle->c = NULL;
	snprintf(handle->ip, sizeof(handle->ip), ip);
	handle->port = port;
	handle->exit = 0;
	sem_init(&handle->sem, 0, 0);
	pthread_create(&handle->work_tid, NULL, work_thread, handle);
	return handle;

err:
	myredis_async_cleanup(handle);
	return NULL;
}

int myredis_async_command(struct myredis_async_handle_t *handle,
		myredis_async_callback_t cbk, void *param,
		const char *fmt, ...)
{
	cmd_t cmd;
	va_list ap;

	LOCK(queue_mtx);
	if (cmd_queue.size() >= QUEUE_MAX_SIZE) {
		UNLOCK(queue_mtx);
		return -1;
	}

	cmd.cbk = cbk;
	cmd.param = param;
	va_start(ap, fmt);
	cmd.len = redisvFormatCommand(&cmd.cmd, fmt, ap); 
	va_end(ap);

	if (cmd.len <= 0) {
		UNLOCK(queue_mtx);
		return -1;
	}

	cmd_queue.push(cmd);
	UNLOCK(queue_mtx);

	sem_post(&handle->sem);
	return 0;
}

int myredis_async_cleanup(struct myredis_async_handle_t *handle)
{
	if (handle == NULL)
		return -1;

	handle->exit = 1;
	sem_post(&handle->sem);
	pthread_join(handle->work_tid, NULL);

	if (handle->c)
		redisFree(handle->c);
	sem_destroy(&handle->sem);
	free(handle);
	return 0;
}
