#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getcfg_v2.h"
#include "strtrim.h"

#define PATH_MAX	512

//int getcfg_v2(char *file, char *sec, char *key, void *val, getcfg_val_t t)
int getcfg_v2(const char *file, const char *sec, const char *key, 
		void *val, getcfg_val_t t)
{
	FILE *fp;
	char line[PATH_MAX], psec[PATH_MAX], pkey[PATH_MAX], pval[PATH_MAX];
	int ret = -1;
        int found_sec = 0;
	int len;

	if ((fp = fopen(file, "r")) == NULL)
		goto out;

	snprintf(psec, sizeof(psec), "[%s]", sec);

	while (fgets(line, sizeof(line), fp) != NULL) {
		strtrim(line);
		if (found_sec == 0) {
			if (strcmp(line, psec) == 0)
				found_sec = 1;
			continue;
		}
		if ((len = strlen(line)) == 0)
			continue;
		if (line[0] == '#')
			continue;
		if (found_sec == 1 && line[0] == '[' && line[len - 1] == ']')
			break;

		if (sscanf(line, "%[^=]=%s", pkey, pval) != 2)
			continue;
		strtrim(pkey);
		strtrim(pval);
		if (strcmp(pkey, key) != 0)
			continue;

		if (t == GETCFG_INT32)
			*(int *)val = atol(pval);
		else if (t == GETCFG_UINT32)
			*(unsigned long *)val = strtoul(pval, NULL, 0);
		else if (t == GETCFG_UINT64)
			*(unsigned long long *)val = strtoull(pval, NULL, 0);
		else
			snprintf((char *)val, strlen(pval) + 1, pval);
		ret = 0;
		break;
	}

out:
	if (fp)
		fclose(fp);
	return ret;
}
