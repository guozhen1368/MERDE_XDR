/*
 * (C) Copyright 2013
 * Beijing HLYT Technology Co., Ltd.
 *
 * adapter_if6svr.h - Adapter interface of if6.
 *
 */

#ifndef _HEAD_ADAPTER_IF6SVR_1A81E618_6BA239D1_237C43EC_H
#define _HEAD_ADAPTER_IF6SVR_1A81E618_6BA239D1_237C43EC_H

#if defined(__cplusplus)
extern "C" {
#endif

extern void *adapter_register_if6svr_cfgfile(char *cfgfile, char *section,
		int (*running)(void));
extern void *adapter_register_if6svr_cfgstr(char *cfgstr, char *section,
		int (*running)(void));
extern void *adapter_register_if6svr(unsigned long cfghd, char *section,
		int (*running)(void));
extern void *adapter_register_if6svr_s(unsigned long cfghd, char *section,
		unsigned int section_index, int channel, int (*running)(void));

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef _HEAD_ADAPTER_IF6SVR_1A81E618_6BA239D1_237C43EC_H */
