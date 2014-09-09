#include "syshdr.h"
#include "getrname.h"

int getrname(char *buff, int buff_size)
{
	char path[PATH_MAX] = "", *p;

	if (readlink("/proc/self/exe", path, sizeof(path)) <= 0)
		return -1;

	if ((p = strrchr(path, FINC_SLASH)) == NULL)
		return -1;

	snprintf(buff, buff_size, p + 1);
	return 0;
}
