#include <stdio.h>
#include <time.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <unistd.h>


#include "util.h"
#include "soapH.h"
#include "porting.h"

char gpLocalAddr[32]={0};
#define LOCAL_ADDR gpLocalAddr//"192.168.2.102"

struct sockaddr_in gMSockAddr;

// Utilities for gSOAP XML handle
char pBuffer[10000]; // XML buffer 
int  vBufLen = 0; 
void clearXmlBuffer(void)
{
	vBufLen = 0;
	memset(pBuffer, 0, sizeof(pBuffer));
}

int mysend(struct soap *soap, const char *s, size_t n) 
{ 
   if (vBufLen + n > sizeof(pBuffer)) 
      return SOAP_EOF; 
   strcpy(pBuffer + vBufLen, s); 
   vBufLen += n; 
   return SOAP_OK; 
} 

char* getXmlBufferData(void)
{
	return pBuffer;
}

void * MyMalloc(int vSize)
{
	char *pSrc = NULL;
	
	if(vSize<=0) return NULL;
		
	pSrc = malloc(vSize);
	memset(pSrc, 0, vSize);
	
	return pSrc;
}

char * CopyString(char *pSrc)
{
	int vLen = 0;
	char *pDst = NULL;
	
	if(!pSrc) return NULL;
		
	vLen = strlen(pSrc);
	pDst = MyMalloc(vLen);	
	memset(pDst, 0, vLen);
	memcpy(pDst, pSrc, vLen);
	
	return pDst;
}


// Utilities of Network
char * getMyIpString(void)
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) 
        { 	
        		// check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            DBG("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
            
            // Note: you may set local address for different interface. For example:eth0, eth1
		        memcpy(gpLocalAddr, addressBuffer, strlen(addressBuffer));
        } 
        else if (ifa->ifa_addr->sa_family==AF_INET6) 
        { 	
        		// check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            DBG("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
        }         
    }
    
    printf("gpLocalAddr is set to %s\n\n", gpLocalAddr);
    
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return gpLocalAddr;
}


int CreateUnicastClient(struct sockaddr_in *pSockAddr)
{
	// http://www.tenouk.com/Module41c.html
	struct in_addr localInterface;
	int sd;
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
	  perror("Opening datagram socket error");
	  exit(1);
	}
	else
	  DBG("Opening the datagram socket...OK.\n");

			
	fprintf(stderr,"sock=%d, s_addr=%s, sin_port=%d\n", sd, inet_ntoa(pSockAddr->sin_addr), htons(pSockAddr->sin_port));	
	
//	memset((char *) &gSockAddr, 0, sizeof(gSockAddr));
//	gSockAddr.sin_family = AF_INET;
//	gSockAddr.sin_addr = pSockAddr->sin_addr;
//	gSockAddr.sin_port = pSockAddr->sin_port;
	  
	return sd;
}


int CreateMulticastClient(int port)
{
	// http://www.tenouk.com/Module41c.html
	struct in_addr localInterface;
	int sd;
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
	  perror("Opening datagram socket error");
	  exit(1);
	}
	else
	  DBG("Opening the datagram socket %d...OK.\n", sd);

	memset((char *) &gMSockAddr, 0, sizeof(gMSockAddr));
	gMSockAddr.sin_family = AF_INET;
	gMSockAddr.sin_addr.s_addr = inet_addr(MULTICAST_ADDR);
	gMSockAddr.sin_port = htons(port);
	  	
	localInterface.s_addr = inet_addr(LOCAL_ADDR);
	if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface.s_addr, sizeof(localInterface)) < 0)
	{
	  perror("Setting local interface error");
	  exit(1);
	}
	else
	  DBG("Setting the local interface...OK\n");  	
	  
	return sd;
}

int CreateMulticastServer(void)
{
	struct ip_mreq group;
	struct sockaddr_in localSock;
	int sd;
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
	  perror("Opening datagram socket error");
	  exit(1);
	}
	else
	  DBG("Opening the datagram socket %d...OK.\n",sd);

	{
		int reuse = 1;
		if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
		{
			perror("Setting SO_REUSEADDR error");
			close(sd);
			exit(1);
		}
		else
			DBG("Setting SO_REUSEADDR...OK.\n");
	}

	memset((char *) &localSock, 0, sizeof(localSock));
	localSock.sin_family = AF_INET;
	localSock.sin_port = htons(MULTICAST_PORT);
	localSock.sin_addr.s_addr = INADDR_ANY;//inet_addr(LOCAL_ADDR);
	if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
	{
		perror("Binding datagram socket error");
		close(sd);
		exit(1);
	}
	else
		DBG("Binding datagram socket...OK.\n");
	  	
	group.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDR);
	group.imr_interface.s_addr = inet_addr(LOCAL_ADDR);
	if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		perror("Adding multicast group error");
		close(sd);
		exit(1);
	}
	else
		DBG("Adding multicast group...OK.\n");

	return sd;
}

int match_rfc3986(char *pItem)
{ 
	char *pSystemScopes = nativeGetScopes();
	if(strstr(pSystemScopes,pItem))
		return 1;
	else 
		return 0;
}

int match_uuid(char *pItem)
{ 
	char *pSystemScopes = nativeGetScopes();
	if(strstr(pSystemScopes,pItem))
		return 1;
	else 
		return 0;
}

int match_ldap(char *pItem)
{ 
	char *pSystemScopes = nativeGetScopes();
	if(strstr(pSystemScopes,pItem))
		return 1;
	else 
		return 0;
}

int match_strcmp0(char *pItem)
{ 
	char *pSystemScopes = nativeGetScopes();
	if(strstr(pSystemScopes,pItem))
		return 1;
	else 
		return 0;
}

int match_none(char *pItem)
{ 
	char *pSystemScopes = nativeGetScopes();
	if(strstr(pSystemScopes,pItem))
		return 1;
	else 
		return 0;
}
							