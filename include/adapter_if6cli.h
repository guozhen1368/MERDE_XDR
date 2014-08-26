/*
 * (C) Copyright 2013
 * Beijing HLYT Technology Co., Ltd.
 *
 * adapter_if6cli.h - A brief description to describe this file.
 *
 */

#ifndef _HEAD_ADAPTER_IF6CLI_917D4DC5_1413BD75_2CCCBA9F_H
#define _HEAD_ADAPTER_IF6CLI_917D4DC5_1413BD75_2CCCBA9F_H

#if defined(__cplusplus)
extern "C" {
#endif

DLL_APP void *adapter_register_if6cli(unsigned long cfghd, char *section);
DLL_APP void *adapter_register_if6cli_cfgfile(char *cfgfile, char *section);
DLL_APP void *adapter_register_if6cli_cfgstr(char *cfgstr, char *section);
DLL_APP void *adapter_register_if6cli_s(unsigned long cfghd, char *section,
		unsigned int section_index);

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef _HEAD_ADAPTER_IF6CLI_917D4DC5_1413BD75_2CCCBA9F_H */
