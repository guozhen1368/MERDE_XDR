#ifndef _FINC_LINUX_H_
#define _FINC_LINUX_H_

#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <pthread.h>
#include <linux/limits.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <libgen.h>		/* basename() */
#include <semaphore.h>
#include <sys/mman.h>
#include <net/if.h>
#include <sys/un.h>		/* struct 'sockaddr_un' */
#include <termios.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <alsa/iatomic.h>	/* atomic_xxx() */

#define FINC_SLASH	'/'

#define close_sock(sock)	close(sock)

#endif	/* _FINC_LINUX_H_ */
