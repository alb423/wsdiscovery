#include <stdio.h>
#include <time.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>

#include "soapH.h"

//#define _DEUBG_
#ifdef _DEUBG_
	#define DBG printf
#else
	#define DBG 
#endif

char gpLocalAddr[32]={0};
#define MULTICAST_ADDR "239.255.255.250"
#define MULTICAST_PORT 3702
#define LOCAL_ADDR gpLocalAddr//"192.168.2.102"

char * CopyString(char *pSrc);
int getMyIp(void);
int CreateMulticastClient(void);
int CreateMulticastServer(void);
int SendHello(int socket);
int SendBye(int socket);

struct sockaddr_in gMSockAddr;
char pBuffer[10000]; // XML buffer 
int  vBufLen = 0; 

   
SOAP_NMAC struct Namespace namespaces[] =
{
   {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
   {"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
   {"wsa2", "http://www.w3.org/2005/03/addressing", NULL, NULL},
   {"dis", "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01", NULL, NULL},
   {NULL, NULL, NULL, NULL}
};


int main(int argc, char **argv)
{
	int socket_cli = 0, socket_srv = 0;
	
	getMyIp();
	
	socket_cli = CreateMulticastClient();
	socket_srv = CreateMulticastServer();
	
	SendHello(socket_cli); usleep(500000);
	//SendHello(socket); usleep(500000);
	//SendHello(socket); usleep(500000);

	// Handle Probe and Resolve request
	#if 0
	{
		struct soap* pSoap = malloc(sizeof(struct soap));
		//pSoap->sendfd
		pSoap->recvfd = socket_srv;
		soap_serve(pSoap);
		
//		if(read(sd, databuf, datalen) < 0)
//		{
//		perror("Reading datagram message error");
//		close(sd);
//		exit(1);
//		}
	}
	#endif 
	
	
	SendBye(socket_cli); usleep(500000);
	//SendBye(socket); usleep(500000);
	//SendBye(socket); usleep(500000);	
}


// Utilities for gSOAP XML handle
char * CopyString(char *pSrc)
{
	int vLen = 0;
	char *pDst = NULL;
	
	if(!pSrc) return NULL;
		
	vLen = strlen(pSrc);
	pDst = malloc(vLen);	
	memset(pDst, 0, vLen);
	memcpy(pDst, pSrc, vLen);
	
	return pDst;
}

void clearBuffer(void)
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


// Send Multicast Packet (Hello and Bye)
int SendHello(int socket)
{
	int vErr = 0;
	struct __wsdd__Hello *pWsdd__Hello = malloc(sizeof(struct __wsdd__Hello));
	struct wsdd__HelloType *pWsdd__HelloType = malloc(sizeof(struct wsdd__HelloType));	
	struct soap *pSoap=malloc(sizeof(struct soap));
	soap_init(pSoap);

	pSoap->fsend = mysend;

	// Build SOAP Header
	pSoap->header = (struct SOAP_ENV__Header *) malloc(sizeof(struct SOAP_ENV__Header));
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Hello");
	pSoap->header->wsa5__MessageID = CopyString("urn:uuid:73948edc-3204-4455-bae2-7c7d0ff6c37c");
	pSoap->header->wsa5__To = CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) malloc(sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = 1077004800;
	pSoap->header->wsdd__AppSequence->MessageNumber = 1;

	// Build Hello Message
	soap_default___wsdd__Hello(pSoap, pWsdd__Hello);
	pSoap->encodingStyle = NULL;
	
	pWsdd__Hello->wsdd__Hello = pWsdd__HelloType;   
	pWsdd__HelloType->wsa5__EndpointReference.Address = CopyString("urn:uuid:98190dc2-0890-4ef8-ac9a-5940995e6119");
	pWsdd__HelloType->Types = CopyString("tds:Device");
	pWsdd__HelloType->Scopes = malloc(sizeof(struct wsdd__ScopesType));
	pWsdd__HelloType->Scopes->MatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");; 
	pWsdd__HelloType->Scopes->__item = CopyString("onvif://www.onvif.org/Profile/Streaming");
	pWsdd__HelloType->XAddrs = CopyString("444");
	pWsdd__HelloType->MetadataVersion = 0;
				   
	
	soap_serializeheader(pSoap);

	soap_response(pSoap, SOAP_OK);
	soap_envelope_begin_out(pSoap);
	soap_putheader(pSoap);
	soap_body_begin_out(pSoap);
	vErr = soap_put___wsdd__Hello(pSoap, pWsdd__Hello, "-wsdd:Hello", "wsdd:HelloType");
	soap_body_end_out(pSoap);
	soap_envelope_end_out(pSoap);

	DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
	
	if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	  DBG("Sending datagram message...OK\n");	
	  
	clearBuffer();
	soap_destroy(pSoap);	  
	return SOAP_OK;
}

int SendBye(int socket)
{
	int vErr = 0;
	struct __wsdd__Bye *pWsdd__Bye = malloc(sizeof(struct __wsdd__Bye));
	struct wsdd__ByeType *pWsdd__ByeType = malloc(sizeof(struct wsdd__ByeType));	
	struct soap *pSoap=malloc(sizeof(struct soap));
	soap_init(pSoap);

	pSoap->fsend = mysend;

	// Build SOAP Header
	pSoap->header = (struct SOAP_ENV__Header *) malloc(sizeof(struct SOAP_ENV__Header));
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Hello");
	pSoap->header->wsa5__MessageID = CopyString("urn:uuid:73948edc-3204-4455-bae2-7c7d0ff6c37c");
	pSoap->header->wsa5__To = CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) malloc(sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = 1077004800;
	pSoap->header->wsdd__AppSequence->MessageNumber = 1;

	// Build Bye Message
	soap_default___wsdd__Bye(pSoap, pWsdd__Bye);
	pSoap->encodingStyle = NULL;
	
	pWsdd__Bye->wsdd__Bye = pWsdd__ByeType;   
	pWsdd__ByeType->wsa5__EndpointReference.Address = CopyString("urn:uuid:98190dc2-0890-4ef8-ac9a-5940995e6119");
	pWsdd__ByeType->Types = CopyString("tds:Device");
	pWsdd__ByeType->Scopes = malloc(sizeof(struct wsdd__ScopesType));
	pWsdd__ByeType->Scopes->MatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");; 
	pWsdd__ByeType->Scopes->__item = CopyString("onvif://www.onvif.org/Profile/Streaming");
	pWsdd__ByeType->XAddrs = CopyString("444");
	pWsdd__ByeType->MetadataVersion = 0;
				   
	
	soap_serializeheader(pSoap);

	soap_response(pSoap, SOAP_OK);
	soap_envelope_begin_out(pSoap);
	soap_putheader(pSoap);
	soap_body_begin_out(pSoap);
	vErr = soap_put___wsdd__Bye(pSoap, pWsdd__Bye, "-wsdd:Bye", "wsdd:ByeType");
	soap_body_end_out(pSoap);
	soap_envelope_end_out(pSoap);

	DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
	
	if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	  DBG("Sending datagram message...OK\n");	
	  
	clearBuffer();
	soap_destroy(pSoap);	  
	return SOAP_OK;
	
}


// Utilities of Network
int getMyIp(void)
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
    return 0;
}


int CreateMulticastClient(void)
{
	// http://www.tenouk.com/Module41c.html
	struct in_addr localInterface;
	
	int sd;
	
	char databuf[1024] = "Multicast test message lol!";
	int datalen = sizeof(databuf);
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
	  perror("Opening datagram socket error");
	  exit(1);
	}
	else
	  DBG("Opening the datagram socket...OK.\n");

	memset((char *) &gMSockAddr, 0, sizeof(gMSockAddr));
	gMSockAddr.sin_family = AF_INET;
	gMSockAddr.sin_addr.s_addr = inet_addr(MULTICAST_ADDR);
	gMSockAddr.sin_port = htons(MULTICAST_PORT);
	  	
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
	
	char databuf[1024] = "Multicast test message lol!";
	int datalen = sizeof(databuf);
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
	  perror("Opening datagram socket error");
	  exit(1);
	}
	else
	  DBG("Opening the datagram socket...OK.\n");

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
	localSock.sin_port = htons(4321);
	localSock.sin_addr.s_addr = INADDR_ANY;
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

// Receive Multicast request and send unicast response (Probe and Resolve)
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap *pSoap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{
	printf("__wsdd__ProbeMatches\n");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap *pSoap, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{
	printf("__wsdd__ResolveMatches\n");
	return SOAP_FAULT;
}


// Unused callback function
SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *pSoap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap *pSoap, struct wsdd__HelloType *wsdd__Hello)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap *pSoap, struct wsdd__ByeType *wsdd__Bye)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Probe(struct soap *pSoap, struct wsdd__ProbeType *wsdd__Probe)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap *pSoap, struct wsdd__ResolveType *wsdd__Resolve)
{return SOAP_FAULT;}