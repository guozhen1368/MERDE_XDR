#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include "syslog.h"

#define LOCK(mtx)	pthread_mutex_lock(&mtx)
#define UNLOCK(mtx)	pthread_mutex_unlock(&mtx)

#define IS_SAMETIME(style, t1, t2) (style == LOG_STYLE_PER_DAY ? \
	((t1).tm_year == (t2).tm_year && \
	 (t1).tm_mon  == (t2).tm_mon && \
	 (t1).tm_mday == (t2).tm_mday) \
	: \
	((t1).tm_year == (t2).tm_year && \
	 (t1).tm_mon  == (t2).tm_mon && \
	 (t1).tm_mday == (t2).tm_mday && \
	 (t1).tm_hour == (t2).tm_hour)) 

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static char rootpath[1024] = {0};
static char filename[512] = {0};
static int  log_style = LOG_STYLE_PER_HOUR;
static FILE *syslog_fp = NULL;
static int  syslog_pri = LOG_DEBUG;		/* default level */
static char fullpath[1024] = {0};
static char fullname[1024] = {0};
static struct tm tm_open;

static inline void get_timestamp(struct timeval *tv, struct tm *tm, 
		char *buff, int buff_size)
{ 
	snprintf(buff, buff_size, "%d-%02d-%02d %02d:%02d:%02d.%03d", 
			tm->tm_year + 1900, 
			tm->tm_mon + 1, 
			tm->tm_mday, 
			tm->tm_hour, 
			tm->tm_min,
			tm->tm_sec,
			(int)(tv->tv_usec / 1000));
}

static void remove_slash(char *path)
{
	int len = strlen(path);
	if (path[len - 1] == '/')
		path[len - 1] = '\0';
}

static void get_fullname(struct tm *tm)
{
	if (log_style == LOG_STYLE_PER_DAY) {
		snprintf(fullpath, sizeof(fullpath),
				"%s/%04d%02d%02d",
				rootpath, 
				tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
	} else if (log_style == LOG_STYLE_PER_HOUR) {
		snprintf(fullpath, sizeof(fullpath),
				"%s/%04d%02d%02d/%02d",
				rootpath, 
				tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
				tm->tm_hour);
	} else {
		snprintf(fullpath, sizeof(fullpath), rootpath);
	}

	snprintf(fullname, sizeof(fullname), "%s/%s", fullpath, filename);
}

static void close_file()
{
	if (syslog_fp != NULL && syslog_fp != stdout) {
		fclose(syslog_fp);
		syslog_fp = NULL;
	}
}

static int create_dir(const char *sFullPath)
{
	char DirName[1024];
	int i;
	int len;

	strcpy(DirName, sFullPath);
	len = strlen(DirName);
	if (len != 0) {
		if (DirName[len-1]!='/')
			strcat(DirName, "/");
	}

	len = strlen(DirName);

	for (i=1; i<len; i++) {
		if (DirName[i]=='/') {
			DirName[i] = 0;
			if (access(DirName, R_OK) != 0) {
				if (mkdir(DirName, 0755)==-1)
					return -1;
			}
			DirName[i] = '/';
		}
	}
	return 0;
}

static int open_file(struct tm *tm)
{
	close_file();

	get_fullname(tm);
	if (create_dir(fullpath) != 0)
		return -1;

	syslog_fp = fopen(fullname, "a+");
	if (syslog_fp == NULL)
		return -1;

	tm_open = *tm;
	setlinebuf(syslog_fp);	/* set stream to line buffered */
	return 0;
}

void openlog(const char *root, const char *logfile_name, 
		int tag_opt, int style,
		int file_size, int rotate_max_count)
{
	time_t tt;
	struct tm tm;

	if (strlen(rootpath) > 0)		/* already open */
		return;

	if (root) {
		snprintf(rootpath, sizeof(rootpath), root);
		remove_slash(rootpath);
	} else
		snprintf(rootpath, sizeof(rootpath), "/var/log");

	if (logfile_name)
		snprintf(filename, sizeof(filename), logfile_name);
	else
		snprintf(filename, sizeof(filename), "TODO_FILENAME.log");

	log_style = style;

	if (tag_opt & LOG_CONS)
		syslog_fp = stdout;
	else {
		tt = time(NULL);
		localtime_r(&tt, &tm);
		if (open_file(&tm) == -1)
			syslog_fp = stdout;
	}
}

void syslog(int pri, const char *fmt, ...)
{
	char hdr[32] = {0};
	struct timeval tv;
	struct tm tm;
	struct stat st;
	va_list ap;

	if (pri > syslog_pri)
		return;

	if (syslog_fp == NULL) {	/* not open syslog yet */
		openlog(NULL, "TODO_FILENAME", LOG_CONS | LOG_PID, 
				LOG_STYLE_GENERAL, 0, 0);
	}

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm);
	get_timestamp(&tv, &tm, hdr, sizeof(hdr));

	LOCK(mtx);

	/* check: need to change directory */
	if (syslog_fp != stdout && log_style != LOG_STYLE_GENERAL &&
			!IS_SAMETIME(log_style, tm, tm_open)) {
		if (open_file(&tm) != 0)
			goto out;
	}

	/* check: file not exist */
	if (syslog_fp != stdout && stat(fullname, &st) != 0) {
		if (open_file(&tm) != 0)
			goto out;
		st.st_size = 0;
	}

#if 0
	/* check: need to rotate file */
	if (syslog_fp != stdout && (unsigned long)st.st_size > maxlen) {
		if (rotate_file(&tm) != 0)
			goto out;
	}
#endif

	fprintf(syslog_fp, "%s ", hdr);
	va_start(ap, fmt);
	vfprintf(syslog_fp, fmt, ap);
	va_end(ap);
	fprintf(syslog_fp, "\n");

out:
	UNLOCK(mtx);
}

void closelog()
{
	LOCK(mtx);
	if (syslog_fp != NULL && syslog_fp != stdout) {
		fclose(syslog_fp);
		syslog_fp = NULL;
	}
	UNLOCK(mtx);
}

int setlogmask(int mask)
{
	int syslog_mask = 0xFF;
	syslog_pri = LOG_DEBUG;

	while (syslog_mask > mask) {
		syslog_mask = (syslog_mask >> 1) - 1;
		syslog_pri--;
	}

	return syslog_pri;
}
