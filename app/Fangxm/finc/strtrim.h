#ifndef _STRTRIM_H_
#define _STRTRIM_H_

/**
 * @brief trim blank space at a string's begin and end. For example, 
 * "  hello world  " will become "hello world"
 * 
 * @param *s the orign string.
 * 
 * @return the string after trim blank space.
 */
char *strtrim(char *s);

#endif	/* _STRTRIM_H_ */
