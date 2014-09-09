#include "syshdr.h"
#include "set_corefile.h"

int set_corefile(const unsigned long long size)
{
#ifdef _LINUX_
	struct rlimit lmt;

	lmt.rlim_cur = lmt.rlim_max = (size == R_UNLIMITED ? 
			RLIM_INFINITY : size);
	if (setrlimit(RLIMIT_CORE, &lmt) != 0)
		return -1;
	system("echo \"core.%e.%p\" > /proc/sys/kernel/core_pattern");
#endif
	return 0;
}
