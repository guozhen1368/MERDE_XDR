#include "syshdr.h"
#include "tcplsn.h"

#define BACKLOG		2048	/* 2nd argument to listen() */

int tcplsn(int port)
{
	int sock;
	struct sockaddr_in addr;
	int reuse = 1;
	int ret;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		goto err;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

#ifdef _WIN32_
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
			(const char *)&reuse, sizeof(int));
#else
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
#endif
	if (ret != 0)
		goto err;

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
		goto err;

	if (listen(sock, BACKLOG) != 0)
		goto err;

	return sock;

err:
	if (sock != -1)
		close_sock(sock);
	return -1;
}
