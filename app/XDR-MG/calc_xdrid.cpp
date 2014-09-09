#include "calc_xdrid.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>        //for struct ifreq
#include <time.h>
#include <unistd.h>

int get_mac(unsigned char * mac)    //返回值是实际写入char * mac的字符个数（不包括'\0'）
{
	struct ifreq ifr[10];
	struct ifconf ifc;
	int sock_fd;

	if ((sock_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror ("socket");
		return -1;
	}

	ifc.ifc_len = sizeof(ifr);
	ifc.ifc_req = ifr;
	if(ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc) < 0)
	{
		perror ("ioctl conf");
	}

	unsigned char m[6];
	memset(m, 0, sizeof m);
	int nu = ifc.ifc_len /sizeof(ifreq);
	for (int i=0; i< nu; i++)
	{
		//printf("nu = %d name=%s  \n", nu, ifr[i].ifr_name);
		if (ioctl (sock_fd, SIOCGIFHWADDR, &ifr[i]) < 0)
		{
			perror ("ioctl");
			return -1;
		}

		
		if (0 != memcmp(m, ifr[i].ifr_hwaddr.sa_data, sizeof m))
		{
			memcpy(mac, ifr[i].ifr_hwaddr.sa_data, 6);
			/*printf("mac = %X:%X:%X:%X:%X:%X\n", 
			(unsigned char)ifr[i].ifr_hwaddr.sa_data[0] ,
			(unsigned char)ifr[i].ifr_hwaddr.sa_data[1] ,
			(unsigned char)ifr[i].ifr_hwaddr.sa_data[2] ,
			(unsigned char)ifr[i].ifr_hwaddr.sa_data[3] ,
			(unsigned char)ifr[i].ifr_hwaddr.sa_data[4] ,
			(unsigned char)ifr[i].ifr_hwaddr.sa_data[5]
			);*/
				 break;
		}
	}

	////unsigned int ip = ((struct sockaddr_in*)&(ifr[index].ifr_addr))->sin_addr;
	//unsigned char * sa_data = (unsigned char *)ifr[index].ifr_hwaddr.sa_data;

	//char buf[18];
	//snprintf (buf, 18, "%X:%X:%X:%X:%X:%X",  sa_data[0], sa_data[1], sa_data[2], sa_data[3], sa_data[4], sa_data[5]);
	//printf("mac = %s\n", buf);

	return	0;
}

GenerateXdrId *GenerateXdrId::single_xdrid_g_ = 0;

GenerateXdrId::GenerateXdrId()
{
	get_mac(xdr_.mac);
	xdr_.tm = (unsigned int)time(NULL);
	xdr_.pid = (unsigned short)getpid();
	xdr_.num = 0;
}

GenerateXdrId * GenerateXdrId::instance()
{
	if (0 == single_xdrid_g_)
	{
		static GenerateXdrId instanc;
		single_xdrid_g_ = &instanc;
	}

	return single_xdrid_g_;
}

void GenerateXdrId::getUniqueXdrid(XDRID & xdrid)
{
	if (xdr_.num > 10000*1000)
	{
		xdr_.tm = (unsigned int)time(NULL);
		xdr_.num = 0;
	}

	memcpy(xdrid.mac, xdr_.mac, sizeof xdr_.mac);
	xdrid.pid = xdr_.pid;
	xdrid.tm = xdr_.tm;
	xdrid.num = xdr_.num++;
}