#ifndef _MYREDIS_ASYNC_H_
#define _MYREDIS_ASYNC_H_

/* asynchronous operate REDIS */

#include <stdarg.h>	/* va_list */

#define QUEUE_MAX_SIZE	10000000

typedef void (*myredis_async_callback_t)(const char *data, 
		const int data_len, void *param);

typedef struct myredis_async_handle_t myredis_async_handle_t;

struct myredis_async_handle_t *myredis_async_init(char *ip, int port);

int myredis_async_command(struct myredis_async_handle_t *handle,
		myredis_async_callback_t cbk, void *param,
		const char *fmt, ...);

int myredis_async_cleanup(struct myredis_async_handle_t *handle);

#endif	/* _MYREDIS_ASYNC_H_ */
