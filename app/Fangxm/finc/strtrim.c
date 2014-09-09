#include <string.h>
#include "strtrim.h"

char *strtrim(char *s)
{
	int len = strlen(s);
	register const unsigned char *p = (const unsigned char *)s;

	enum {
		STAT_BLANK,
		STAT_LETTER
	} stat = STAT_BLANK;

	int start = 0, end = 0, pos = 0;
	char c;

	while ((c = *p++)) {
		if (c != ' ' && c != '\t' && c != '\n') {
			if (stat == STAT_BLANK) {
				stat = STAT_LETTER;
				start = pos;
			} else if (stat == STAT_LETTER)
				end = pos;
		}
		pos++;
	}

	if (end == 0)
		end = start;

	if (start != 0 || end != len) {
		p = (const unsigned char *)s;
		memcpy(s, &p[start], end - start + 1);
		s[end - start + 1] = '\0';
	}
	return s;
}
