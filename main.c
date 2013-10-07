#include <stdio.h>
#include <time.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <unistd.h>

#include <pthread.h>
#include <sys/msg.h>

#include "soapH.h"
#include "util.h"
#include "mySoapStub.h"
#include "porting.h"
                 	   

#define PERMS 0666
#define ONVIF_DIS_MSG_KEY					((key_t) 88888L)
#define ONVIF_MSG_UPDATE_SCOPES		1919L
#define ONVIF_MSG_REBOOT					1920L
#define ONVIF_MSG_DISCOVERYMODE		1921L

typedef struct native_msg_buf
{
	long mtype;
	char mtext[4];
} native_msg_buf;

                 	   
int thread_ret=0, thread_no=0;
typedef struct {
	pthread_t thread_tid;
	long thread_count;
}tThread;
tThread tptr[2];

void send_msg_update_scopes();
void send_msg_reboot();
void send_msg_discoverymode(char vMode);
void RecvThread(void* data);

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
		// If the executable name is ws-client
		if(strcmp(&argv[0][vExecutableLen-vLen],"ws-client")==0)
		{
			return _client(argc, argv);
		}
		else
		{
			return _server(argc, argv);
		}		
	}
	return 1;
}

int _server(int argc, char **argv)
{
	int msocket_cli = 0, msocket_cli2 = 0, msocket_srv = 0, msocket_srv2 = 0;	
	char *pAddress=NULL, *pAddressWifi=NULL;
	struct soap* pSoap = NULL;

  initMyIpString();
	
	// For MAC only
	pAddress = getMyIpString(INTERFACE_NAME_1);
	pAddressWifi = getMyIpString(INTERFACE_NAME_2);
	//pAddress = getMyIpString("eth0");
	//pAddressWifi = getMyIpString("eth1");
		
	InitMyRandom(pAddress);
	
		
	if(pAddress)
	{
		msocket_cli = CreateMulticastClient(pAddress, MULTICAST_PORT);
	}
	if(pAddressWifi)
	{		
		msocket_cli2 = CreateMulticastClient(pAddressWifi, MULTICAST_PORT);
	}
	

			
	msocket_srv = CreateMulticastServer();
	pSoap = MyMalloc(sizeof(struct soap));
	soap_init1(pSoap, SOAP_IO_UDP);
			
	//pSoap->sendfd = msocket_cli;
	//pSoap->recvfd = msocket_srv;
	pSoap->recvsk = msocket_srv;

	usleep(50000);SendHello(msocket_cli); 
	usleep(50000);SendHello(msocket_cli2); 
	usleep(50000);SendHello(msocket_cli); 
	usleep(50000);SendHello(msocket_cli2);
	usleep(50000);SendHello(msocket_cli); 
	usleep(50000);SendHello(msocket_cli2);	 	
		
	thread_ret=pthread_create( &tptr[thread_no].thread_tid, NULL, (void *) RecvThread, (void*)thread_no );
	if(thread_ret!=0)
	{
		fprintf (stderr, "Create pthread error!\n");
		exit (1);
	}
	thread_no++;
         			
	while(1)
	{
		soap_serve(pSoap);
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

	close(msocket_srv);
	return 1;
}

int _client(int argc, char **argv)
{
	int vLen = 0, vExecutableLen = 0;
	int msocket_cli = 0, msocket_cli2= 0;
	char *pAddress=NULL, *pAddressWifi=NULL;
	int vDataLen = 1024*5;
	char pDataBuf[1024*5];
	
	initMyIpString();
	
	pAddress = getMyIpString(INTERFACE_NAME_1);
	pAddressWifi = getMyIpString(INTERFACE_NAME_2);
	
	if(pAddress)
		msocket_cli = CreateMulticastClient(pAddress, MULTICAST_PORT);
	if(pAddressWifi)
		msocket_cli2 = CreateMulticastClient(pAddressWifi, MULTICAST_PORT);
	
	usleep(500000);
	if(argc>=2)
	{
		if(strcmp(argv[1],"1")==0)
		{
			SendHello(msocket_cli); 
		}
		else if(strcmp(argv[1],"2")==0)
		{
			int vReciveLen=0;
			struct sockaddr_in vxSockAddr;
			SOAP_SOCKLEN_T vSockLen = (SOAP_SOCKLEN_T)sizeof(vxSockAddr);
			memset((void*)&vxSockAddr, 0, sizeof(vxSockAddr));

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
			SendBye(msocket_cli); 
		}
		else if(strcmp(argv[1],"5")==0)
		{	
			SendProbeMatches(msocket_cli, &gMSockAddr, NULL);
		}					
		else if(strcmp(argv[1],"6")==0)
		{
			SendResolveMatches(msocket_cli, &gMSockAddr, NULL); 
		}			
		else if(strcmp(argv[1],"u")==0)
		{
			send_msg_update_scopes();		
		}	
		else if(strcmp(argv[1],"r")==0)
		{
			send_msg_reboot();		
		}	
		else if(strcmp(argv[1],"d")==0)
		{
			if(argc>2)
			{
				if(strcmp(argv[2],"0")==0)
					send_msg_discoverymode('0');	
				else
					send_msg_discoverymode('1');	
			}
			else
			{
				send_msg_discoverymode('1');
			}				
		}			
		else if(strcmp(argv[1],"q")==0)
		{
			SendFault(msocket_cli, &gMSockAddr); 
		}																						
	}
	else
	{
		SendHello(msocket_cli); 
	}
	close(msocket_cli);
	return 1;
}


void RecvThread(void* data)
{
	int i;
	int msqid;
	int msocket_cli = 0, msocket_cli2 = 0;
	char *pAddress=NULL, *pAddressWifi=NULL;    
	pthread_detach(pthread_self());
    
  pAddress = getMyIpString(INTERFACE_NAME_1);
  pAddressWifi = getMyIpString(INTERFACE_NAME_2);
  if(pAddress)
		msocket_cli = CreateMulticastClient(pAddress, MULTICAST_PORT);
  if(pAddressWifi)
		msocket_cli = CreateMulticastClient(pAddressWifi, MULTICAST_PORT);
			    
  DBG("RecvThread start....\n");
	if((msqid = msgget(ONVIF_DIS_MSG_KEY, PERMS | IPC_CREAT)) >= 0)
	{
		native_msg_buf  recvmsg ;
		while(1)
		{                    
			DBG("start recv msg .. \n");
			if(msgrcv(msqid, &recvmsg, 4, 0, 0) > 0)
			{
				if(recvmsg.mtype == ONVIF_MSG_UPDATE_SCOPES)
				{
					nativeIncreaseMetadataVersion();
					SendHello(msocket_cli);
					SendHello(msocket_cli);
					SendHello(msocket_cli);
				} 
				else if(recvmsg.mtype == ONVIF_MSG_REBOOT)
				{
					for (i=0;i<2;i++)
					{
						sleep(2);
						SendBye(msocket_cli);
						SendBye(msocket_cli);
						SendBye(msocket_cli);
					}
				} 
				else if(recvmsg.mtype == ONVIF_MSG_DISCOVERYMODE)
				{
					//initDiscoveryMode();
					DBG("recvmsg.mtext[0]=%c\n",recvmsg.mtext[0]);
					nativeChangeDiscoveryMode(recvmsg.mtext[0]);
					SendHello(msocket_cli);
				}
				usleep(50);
			}
			sleep(1);
		}
		
		if(msgctl(msqid, IPC_RMID, (struct msqid_ds *) 0) < 0)
		{
			DBG("msg queue remove error !! \n");
		}
	}       
  
  close(msocket_cli);
  DBG("RecvThread end....\n");
  pthread_exit ("thread all done");
  
}

void send_msg_update_scopes()
{
	int msqid;
  if((msqid = msgget(ONVIF_DIS_MSG_KEY, PERMS | IPC_CREAT)) >= 0)
  {
  	native_msg_buf  sendmsg ;
  	sendmsg.mtype = ONVIF_MSG_UPDATE_SCOPES;
  	if(msgsnd(msqid, &sendmsg, 1, 0) < 0)
  	{
  		perror("msgsnd() error!!\n");
  	}
  }
}

void send_msg_reboot()
{
	int msqid;
  if((msqid = msgget(ONVIF_DIS_MSG_KEY, PERMS | IPC_CREAT)) >= 0)
  {
  	native_msg_buf  sendmsg ;
  	sendmsg.mtype = ONVIF_MSG_REBOOT;
  	if(msgsnd(msqid, &sendmsg, 1, 0) < 0)
  	{
  		perror("msgsnd() error!!\n");
  	}
  }
}

  
void send_msg_discoverymode(char vMode)
{
	int msqid;
  if((msqid = msgget(ONVIF_DIS_MSG_KEY, PERMS | IPC_CREAT)) >= 0)
  {
  	native_msg_buf  sendmsg ;
  	sendmsg.mtype = ONVIF_MSG_DISCOVERYMODE;
  	sendmsg.mtext[0] = vMode;
  	DBG("\n send_msg_discoverymode() vMode = %c \n\n", vMode);
  	if(msgsnd(msqid, &sendmsg, 1, 0) < 0)
  	{
  		perror("msgsnd() error!!\n");
  	}
  }
}
