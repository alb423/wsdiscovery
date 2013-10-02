#ifndef UTIL_H
#define UTIL_H

#include "soapH.h"
#include <netinet/in.h>
#include <ifaddrs.h>

#define _DEUBG_
#ifdef _DEUBG_
	#define DBG printf
#else
	#define DBG 
#endif

#define MULTICAST_ADDR "239.255.255.250"
#define MULTICAST_PORT 3702

// Network
extern struct sockaddr_in gMSockAddr;
extern int getMyIp(void);
extern int CreateMulticastClient(int port);
extern int CreateMulticastServer(void);
extern int CreateUnicastClient(struct sockaddr_in *pSockAddr);

// Xml send callback
extern void clearXmlBuffer(void);
extern int mysend(struct soap *soap, const char *s, size_t n);
extern char* getXmlBufferData(void);


extern void * MyMalloc(int vSize);
extern char * CopyString(char *pSrc);

#endif

