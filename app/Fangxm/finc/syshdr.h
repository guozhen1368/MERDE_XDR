#ifndef _SYSHDR_H_
#define _SYSHDR_H_

#if !defined(_LINUX_) && !defined(_WIN32_)
#define _LINUX_		/* default with linux */
#endif

/* normal system header files. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_LINUX_)
#include "finc_linux.h"
#elif defined(_WIN32_)
#include "finc_win32.h"
#endif

#define SEM_WAIT(p_sem, p_retval) \
	do { \
		while ((*p_retval = sem_wait(p_sem)) == -1 \
				&& errno == EINTR) \
			continue; \
	} while (0)

#define SEM_TIMEDWAIT(p_sem, p_abs_timeout, p_retval) \
	do { \
		while ((*p_retval = sem_timedwait(p_sem, p_abs_timeout) == -1) \
				&& errno == EINTR) \
			continue; \
	} while (0)

#endif	/* _SYSHDR_H_ */
