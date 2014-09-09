#include "syshdr.h"
#include "recvn.h"

int recvn(int sockfd, void *buf, int n, int flag)
{
	int readed = 0, left = n, bytes;

	while (left > 0) {
		bytes = recv(sockfd, (char *)buf + readed, left, flag);
		if (bytes == -1 && errno == EINTR)
			continue;
		if (bytes <= 0)
			return bytes;

		readed += bytes;
		left -= bytes;
	}

	return n;
}
