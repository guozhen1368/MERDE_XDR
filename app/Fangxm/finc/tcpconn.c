#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "tcpconn.h"

int tcpconn(const char *ip, int port, int timeout)
{
	int sock;
	struct sockaddr_in addr;
	struct timeval tmo;
	int reuse = 1;
	int old_flags = 0;

	if (ip == NULL || port <= 0)
		return -1;

	/* create a socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		goto err;

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
		goto err;

	if (timeout > 0) {
		tmo.tv_sec = timeout;
		tmo.tv_usec = 0;

		old_flags = fcntl(sock, F_GETFL);
		if (old_flags == -1)
			goto err;
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tmo, sizeof(tmo));
	}

	/* connect to server */
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		goto err;

	if (timeout > 0) {
		if (fcntl(sock, F_SETFL, old_flags) == -1)
			goto err;
	}

	return sock;

err:
	if (sock != -1)
		close(sock);
	return -1;
}
