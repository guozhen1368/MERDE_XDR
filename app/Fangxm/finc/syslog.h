#ifndef _SYSLOG_H_
#define _SYSLOG_H_

#include <stdarg.h>

/* log directory style */
enum LOG_STYLE {
	LOG_STYLE_GENERAL = 0, 	/* not change directory */
	LOG_STYLE_PER_DAY,	/* change directory every day */
	LOG_STYLE_PER_HOUR	/* change directory every hour */
};

#define LOG_SUFFIXNAME	".log"

/* priorities */
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */

/* arguments to setlogmask */
#define	LOG_MASK(pri)	(1 << (pri))		/* mask for one priority */
#define	LOG_UPTO(pri)	((1 << ((pri)+1)) - 1)	/* all priorities through pri */

/* option flags for openlog */
#define	LOG_PID		0x01	/* log the pid with each message */
#define	LOG_CONS	0x02	/* log on the console if errors in sending */
#define	LOG_ODELAY	0x04	/* delay open until first syslog() (default) */
#define	LOG_NDELAY	0x08	/* don't delay open */
#define	LOG_NOWAIT	0x10	/* don't wait for console forks: DEPRECATED */
#define	LOG_PERROR	0x20	/* log to stderr as well */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 
 * 
 * @param *root_dir specifity the root directory, if NULL, use /var/log
 * @param *file_name specifity the log file name
 * @param tag_opt e.g. LOG_CONS | LOG_PID (note: if have LOG_CONS, it will
 * only output on console.
 * 
 * @return none
 */
void openlog(const char *root_dir, const char *logfile_name, 
		int tag_opt, int log_style, 
		int file_size, int rotate_max_count);
void syslog(int pri, const char *fmt, ...);
void closelog();
int  setlogmask(int mask);

#ifdef __cplusplus
}
#endif

#endif	/* _SYSLOG_H_ */
