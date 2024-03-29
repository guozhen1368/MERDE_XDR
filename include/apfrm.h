/*
 * (C) Copyright 2007
 * Hu Chunlin <chunlin.hu@gmail.com>
 *
 * apfrm.h - A brief description goes here.
 *
 */

#ifndef _HEAD_APFRM_06C11BD6_6363025A_015040E5_H
#define _HEAD_APFRM_06C11BD6_6363025A_015040E5_H

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
#define siginfo_t void
#endif

struct ap_framework {
	char *pidfile;

	int (*run)(long instance, unsigned long data);
	unsigned long  data;

	void (*sigalrm_handle)(void);
	void (*sigusr1_handle)(void);
	void (*sigusr2_handle)(void);

	void (*usage)(char *progname);
	void (*version)(char *progname);
	int (*parse_args)(int argc, char **argv);
};

#if defined(__cplusplus)
extern "C" {
#endif

extern DLL_APP void ap_set_foreground(void);
extern DLL_APP void ap_inc_loglevel(void);
extern DLL_APP void ap_dec_loglevel(void);
extern DLL_APP int ap_is_running(void);
extern DLL_APP void ap_stop_running(void);

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef _HEAD_APFRM_06C11BD6_6363025A_015040E5_H */
