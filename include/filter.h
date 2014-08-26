/*
 * (C) Copyright 2009
 * Beijing HLYT Technology Co., Ltd.
 *
 * filter.h
 *
 */

#ifndef _HEAD_FILTER_1B49230D_52E00F49_5AF0C50F_H
#define _HEAD_FILTER_1B49230D_52E00F49_5AF0C50F_H

#ifndef DLL_APP
#ifdef WIN32
#ifdef _USRDLL
#define DLL_APP _declspec(dllexport)
#else
#define DLL_APP _declspec(dllimport)
#endif
#else
#define DLL_APP
#endif
#endif

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

typedef struct FILTER {
	unsigned char  type;
	unsigned char  bracket;
	unsigned char  non;
	unsigned short option_type;
	unsigned char  op;
	unsigned char  value_type;
	unsigned int   value_len;
	unsigned char *value;
	void          *regex;
	struct FILTER *lchild;
	struct FILTER *rchild;
} sFilter;

#include "filter_option.h"

enum {
	DATATYPE_BYTE1 = 1,
	DATATYPE_BYTE2,
	DATATYPE_BYTE4,
	DATATYPE_BYTE8,
	DATATYPE_CHAR_STRING,
	DATATYPE_OCTET_STRING,
	DATATYPE_SET,
};

enum {
	OP_EQ = 1,
	OP_NEQ,
	OP_LT,
	OP_LE,
	OP_GT,
	OP_GE,
	OP_MATCH,
	OP_STRCMP,    /* Precise comparision: compare both length and content. */
	OP_STRNCMP,
	OP_OCTCMP,
	OP_OCTNCMP,
	OP_IN,
};

#if defined(__cplusplus)
extern "C" {
#endif

DLL_APP void *filter_single(enum filter_option type, unsigned char op,
				unsigned char datatype, void *value, void *value_len);
DLL_APP void *filter_or(void *fc1, void *fc2, int release);
DLL_APP void *filter_and(void *fc1, void *fc2, int release);
DLL_APP void *filter_non(void *filter);
DLL_APP void *filter_copy(void *filter);

DLL_APP void filter_set_bracket(void *filter);

DLL_APP unsigned char *filter_encode(void *filter, int reserved, int *len);
DLL_APP int filter_encode_ext(void *filter, unsigned char *buf, int size);
DLL_APP void *filter_decode(unsigned char *buf, int len);
DLL_APP void  filter_release(void *fc);

DLL_APP int filter_traversing(void *filter, void *arg,
				void (*callback)(enum filter_option type, void *arg));
DLL_APP int filter_traverse(void *filter, void *arg,
				void (*callback)(sFilter *f, void *arg));

DLL_APP int filter_logic_result(void *filter, void *arg,
			int (*compare)(void *arg, sFilter *single, void *value_ex));

DLL_APP int filter_compare_single(void *value, unsigned char datatype,
		        void *single, void *value_ex);

DLL_APP char *filter_get_datatype(unsigned char datatype);
DLL_APP char *filter_get_oper(unsigned char op);
DLL_APP char *filter_get_type(enum filter_option type);
DLL_APP char *filter_get_expression(void *filter, char *buf, int size);

DLL_APP void *filter_parse(char *buf);
DLL_APP int filter_cfghd_s(unsigned long cfghd, char *section, int section_index, void **filter);
DLL_APP int filter_cfgfile_s(char *cfgfile, char *section, int section_index, void **filter);
DLL_APP int filter_cfgstr_s(char *cfgstr, char *section, int section_index, void **filter);
DLL_APP int filter_cfghd(unsigned long cfghd, char *section, void **filter);
DLL_APP int filter_cfgfile(char *cfgfile, char *section, void **filter);
DLL_APP int filter_cfgstr(char *cfgstr, char *section, void **filter);

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef _HEAD_FILTER_1B49230D_52E00F49_5AF0C50F_H */
