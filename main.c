#include <stdio.h>
#include <time.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <unistd.h>

#include "soapH.h"
#include "util.h"
#include "mySoapStub.h"
   
SOAP_NMAC struct Namespace namespaces[] =
{
   {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
   {"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
   {"wsa2", "http://www.w3.org/2005/03/addressing", NULL, NULL},
   {"wsa5", "http://www.w3.org/2005/08/addressing", NULL, NULL},
   {"wsdd", "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01", NULL, NULL},
   {NULL, NULL, NULL, NULL}
};

int _client(int argc, char **argv);
int _server(int argc, char **argv);

int main(int argc, char **argv)
{
	int vLen = 0, vExecutableLen = 0;
		
	// busybox
	vLen = strlen("ws-client");
	vExecutableLen = strlen(argv[0]);
	if(vLen <= vExecutableLen )
	{
		fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
		// If the executable name is ws-client
		if(strcmp(&argv[0][vExecutableLen-vLen],"ws-client")==0)
		{
			return _client(argc, argv);
		}
		else
		{
			fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
			return _server(argc, argv);
		}		
	}
	return 1;
}

int _server(int argc, char **argv)
{
	int msocket_cli = 0, msocket_srv = 0;	
	struct soap* pSoap = NULL;
	
	getMyIp();	
	msocket_cli = CreateMulticastClient(MULTICAST_PORT);
	msocket_srv = CreateMulticastServer();
	
	// Handle Probe and Resolve request
	pSoap = MyMalloc(sizeof(struct soap));
	soap_init1(pSoap, SOAP_IO_UDP);
	
	//pSoap->sendfd = msocket_cli;
	//pSoap->recvfd = msocket_srv;
	pSoap->recvsk = msocket_srv;
			
	while(1)
	{
		soap_serve(pSoap);
		//fprintf(stderr, "%s %s :%d pid=%d \n",__FILE__,__func__, __LINE__, getpid());
		fprintf(stderr, "%s %s :%d peer addr=%s, port=%d, error=%d\n",__FILE__,__func__, __LINE__, inet_ntoa(pSoap->peer.sin_addr), pSoap->peer.sin_port,  pSoap->error);	
		if(pSoap->header)
		{
			fprintf(stderr, "%s %s :%d Action = %s\n",__FILE__,__func__, __LINE__, pSoap->header->wsa5__Action);
			if(strstr(pSoap->header->wsa5__Action,"fault")!=NULL)
			{
				// exit the loop when receive fault
				break;
			}
		}
	}
	close(msocket_cli);
	close(msocket_srv);
	return 1;
}

int _client(int argc, char **argv)
{
	int vLen = 0, vExecutableLen = 0;
	int msocket_cli = 0, msocket_srv = 0;
	int vDataLen = 1024*5;
	char pDataBuf[1024*5];
	
	getMyIp();
	msocket_cli = CreateMulticastClient(MULTICAST_PORT);
	if(argc==2)
	{
		if(strcmp(argv[1],"1")==0)
		{
			usleep(500000);
			SendHello(msocket_cli); 
		}
		else if(strcmp(argv[1],"2")==0)
		{
			int vReciveLen=0;
			struct sockaddr_in vxSockAddr;
			SOAP_SOCKLEN_T vSockLen = (SOAP_SOCKLEN_T)sizeof(vxSockAddr);
			memset((void*)&vxSockAddr, 0, sizeof(vxSockAddr));
			
			
			usleep(500000);
			SendProbe(msocket_cli); 
			
			
			fprintf(stderr, "Waiting Response\n");
			
			vDataLen = sizeof(pDataBuf);
			vReciveLen = recvfrom(msocket_cli, pDataBuf, (SOAP_WINSOCKINT)vDataLen, MSG_WAITALL, (struct sockaddr*)&vxSockAddr, &vSockLen);
			fprintf(stderr, "got Response, vReciveLen=%d, \n", vReciveLen);
			fprintf(stderr, "msocket_cli=%d, rsp s_addr=%s, rsp sin_port=%d\n", msocket_cli, inet_ntoa(vxSockAddr.sin_addr), htons(vxSockAddr.sin_port));
			fprintf(stderr, "Buf=%s\n", pDataBuf);
		}					
		else if(strcmp(argv[1],"3")==0)
		{
			int vErr=0;
			struct sockaddr_in vxSockAddr;
			SOAP_SOCKLEN_T vSockLen = (SOAP_SOCKLEN_T)sizeof(vxSockAddr);
			memset((void*)&vxSockAddr, 0, sizeof(vxSockAddr));
			
			
			usleep(500000);
			SendResolve(msocket_cli); 
			
			
			fprintf(stderr, "Waiting Response\n");
			
			vDataLen = sizeof(pDataBuf);
			vErr = recvfrom(msocket_cli, pDataBuf, (SOAP_WINSOCKINT)vDataLen, MSG_WAITALL, (struct sockaddr*)&vxSockAddr, &vSockLen);
			fprintf(stderr, "got Response, vErr=%d, \n", vErr);
			fprintf(stderr, "msocket_cli=%d, rsp s_addr=%s, rsp sin_port=%d\n", msocket_cli, inet_ntoa(vxSockAddr.sin_addr), htons(vxSockAddr.sin_port));
			fprintf(stderr, "Buf=%s\n", pDataBuf);					
		}					
		else if(strcmp(argv[1],"4")==0)
		{
			usleep(500000);
			SendBye(msocket_cli); 
		}
		else if(strcmp(argv[1],"5")==0)
		{	
			usleep(500000);
			SendProbeMatches(msocket_cli, &gMSockAddr);
		}					
		else if(strcmp(argv[1],"6")==0)
		{
			usleep(500000);
			SendResolveMatches(msocket_cli, &gMSockAddr); 
		}			
		else if(strcmp(argv[1],"7")==0)
		{
			usleep(500000);
			SendFault(msocket_cli); 
		}																
	}
	else
	{
		SendHello(msocket_cli); usleep(500000);
	}
	close(msocket_cli);
	return 1;

}
