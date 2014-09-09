#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "myredis_async.h"

static int no = 0;

static void get_callback(const char *data, const int data_len, void *param)
{
	printf("NO:%d len:%d [%s]\n", ++no, data_len, data);
}

int main(int argc, char **argv)
{
	struct myredis_async_handle_t *handle = NULL;
	int ret = -1;

	char *ip = (argc > 1) ? argv[1] : (char *)"127.0.0.1";
	int port = (argc > 2) ? atoi(argv[2]) : 6379;

	handle = myredis_async_init(ip, port);
	if (handle == NULL) {
		printf("myredis_async_init() failed\n");
		goto out;
	}
	printf("myredis_async_init() success\n");

	while (1) {
		ret = myredis_async_command(handle, NULL, NULL,
				"SET %b %s", "foo", 3, "bar");
		//printf("SET return %d\n", ret);

		ret = myredis_async_command(handle, get_callback, NULL,
				"GET %b", "foo", 3);
		//printf("GET return %d\n", ret);
	}

	ret = 0;
out:
	myredis_async_cleanup(handle);
	return ret;
}
