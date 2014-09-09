#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "getcfg_v2.h"
#include "syslog.h"
#include "set_corefile.h"

#include "xdr_handler.h"
#include "xdr_proc.h"
#include "xdr_recv.h"
#include "xdr_sendto_app.h"
#include "xdr_sendto_merge.h"

static int handle_mode = XDR_MODE_U;
static char log_root_dir[512] = "LOG";
static int log_console = 0;

static void get_opt(int argc, char **argv)
{
	int opt, debug = 0, exit_code = 0;

	while ((opt = getopt(argc, argv, "m:dcvh")) != -1) {
		switch (opt) {
			case 'm':
				if (strcmp(optarg, "xdru") == 0)
					handle_mode = XDR_MODE_U;
				else if (strcmp(optarg, "xdrs") == 0)
					handle_mode = XDR_MODE_S;
				else {
					printf("not support handle mode: %s\n",
							optarg);
					exit_code = -1;
					goto usage;
				}
				break;
			case 'd':
				debug = 1;
				log_console = LOG_CONS;
				break;
			case 'c':
				set_corefile(R_UNLIMITED);
				break;
			case 'v':
				if (argc != 2) {
					exit_code = -1;
					goto usage;
				}
				printf("xdr_handler version:%s build:%s %s\n", 
						VERSION, __DATE__, __TIME__);
				exit(0);
			case 'h':
				if (argc != 2)
					exit_code = -1;
				goto usage;
			default:
				exit_code = -1;
				goto usage;
		}
	}

	if (!debug)
		daemon(1, 0);

	////////////////
	set_corefile(R_UNLIMITED);
	////////////////
	return;

usage:
	printf("Usage: %s [options]\n"
			"\t-m: hadler mode(xdru or xdrs)\n"
			"\t-d: print debug message on console\n"
			"\t-c: dump core file\n"
			"\t-v: print version\n"
			"\t-h: show help\n", argv[0]);
	exit(exit_code);
}

static void uninit(int sig)
{
	syslog(LOG_INFO, "xdr_handler closed");
	closelog();
	exit(0);
}

static void init()
{
	/* set some signal handling */
	signal(SIGTERM, uninit);
	signal(SIGINT, uninit);
	signal(SIGPIPE, SIG_IGN);
}

static int getcfg()
{
	getcfg_v2(CFGFILE, "global", "log_root_dir", log_root_dir, GETCFG_STR);
	return 0;
}

int main(int argc, char **argv)
{
	char *logfile;

	get_opt(argc, argv);
	getcfg();

	if (handle_mode == XDR_MODE_U)
		logfile = (char *)"xdr_handler_u.log";
	else
		logfile = (char *)"xdr_handler_s.log";

	openlog(log_root_dir, logfile, log_console | LOG_PID, 
			LOG_STYLE_PER_HOUR, 0, 10);

	syslog(LOG_INFO, "xdr_handler start ...");
	syslog(LOG_INFO, "version:%s build:%s %s", 
			VERSION, __DATE__, __TIME__);
	syslog(LOG_INFO, "handle mode: %s", handle_mode == XDR_MODE_U ?
			"xdru" : "xdrs");

	init();

	if (xdrproc_start(handle_mode) != 0) {
		syslog(LOG_ERR, "xdrproc_start() failed.");
		return -1;
	}

	if (xdrrecv_start(handle_mode) != 0) {
		syslog(LOG_ERR, "xdrrecv_start() failed.");
		return -1;
	}

	if (xdrs2app_start(handle_mode) != 0) {
		syslog(LOG_ERR, "xdrs2app_start() failed.");
		return -1;
	}

	if (xdrs2m_start(handle_mode) != 0) {
		syslog(LOG_ERR, "xdrs2m_start() failed.");
		return -1;
	}

	while (1)
		pause();
	return 0;
}
