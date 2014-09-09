#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "hlyt_hdr.h"
#include "xdr_save.h"

static char rootpath[1024] = "/tmp/XDR";
static char suffix[24] = ".dat";
static unsigned long filesz = 1024 * 1024 * 1024;	/* 100Mbyte */
static int fd = -1;

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

static void close_file()
{
	if (fd != -1) {
		close(fd);
		fd = -1;
	}
}

static int open_file()
{
	char fullname[1024];
	struct timeval tv;
	struct tm tm;

	close_file();

	if (create_dir(rootpath) != 0)
		return -1;

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm);

	/* /tmp/XDR/xdr-20140903_181901.dat */
	snprintf(fullname, sizeof(fullname), 
			"%s/xdr-%04d%02d%02d_%02d%02d%02d%s", 
			rootpath,
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			suffix);
	fd = open(fullname, O_WRONLY | O_CREAT, 0755);
	if (fd == -1)
		return -1;
	return 0;
}

int xdr_save_init(unsigned long cfghd)
{
	char *sname = (char *)"Data.Archive";
	int size;

	if (CfgGetValue(cfghd, sname, "basename", rootpath, 1, 1) != 0) 
		LOGINFO("Get Data.Archive:basename error, use: %s", rootpath);
	if (CfgGetValue(cfghd, sname, "suffix", suffix, 1, 1) != 0) 
		LOGINFO("Get Data.Archive:suffix error, use: %s", suffix);
	if ((size = CfgGetValueInt(cfghd, sname, "filesz", 1, 1)) != -1)
		filesz = size * 1024 * 1024;
	return 0;
}

int xdr_save(char *xdr, int bytes)
{
	struct stat st;

	if (fstat(fd, &st) != 0) {
		if (open_file() != 0)
			return -1;
		st.st_size = 0;
	}

	if ((unsigned long)st.st_size > filesz) {
		if (open_file() != 0)
			return -1;
	}

	if (write(fd, xdr, bytes) != bytes)
		return -1;
	return 0;
}
