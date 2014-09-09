#include "syshdr.h"
#include "set_keepalive.h"

#if defined(_LINUX_)
#include <netinet/tcp.h>
#endif

#if defined(_WIN32_)
/* Argument structure for SIO_KEEPALIVE_VALS */
typedef struct {
	unsigned long  onoff;
	unsigned long  idel;
	unsigned long  interval;
} tcp_keepalive_t;

#define SIO_KEEPALIVE_VALS	_WSAIOW(IOC_VENDOR, 4)
#endif

int set_keepalive(int sock, int idel, int count, int interval)
{
#if defined(_WIN32_)
	tcp_keepalive_t in_val = {0};
	tcp_keepalive_t out_val = {0};
	unsigned long return_bytes = 0;

	in_val.onoff = 1;
	in_val.interval = interval * 1000;
	in_val.idel = idel * 1000;

	if (WSAIoctl(sock, SIO_KEEPALIVE_VALS,
		(LPVOID)&in_val, sizeof(tcp_keepalive_t),
		(LPVOID)&out_val, sizeof(tcp_keepalive_t),
		&return_bytes, NULL, NULL) == SOCKET_ERROR)
		return -1;

#elif defined(_LINUX_)
	int k = 1;
	setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &k, sizeof(int));
	setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, &idel, sizeof(int));
	setsockopt(sock, SOL_TCP, TCP_KEEPCNT, &count, sizeof(int));
	setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(int));
#endif

	return 0;
}
