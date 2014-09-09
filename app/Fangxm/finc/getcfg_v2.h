#ifndef _GETCFG_V2_H_
#define _GETCFG_V2_H_

/* value type */
typedef enum {
	GETCFG_INT32,	/* int */
	GETCFG_UINT32,	/* unsigned int */
	GETCFG_UINT64,	/* unsigned long long */
	GETCFG_STR	/* string */
} getcfg_val_t;

int getcfg_v2(const char *file, const char *sec, const char *key, 
		void *val, getcfg_val_t t);

#endif	/* _GETCFG_V2_H_ */
