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
#define ONVIF_DIS_MSG_KEY           ((key_t) 88888L)
#define ONVIF_MSG_UPDATE_SCOPES     1919L
#define ONVIF_MSG_REBOOT            1920L
#define ONVIF_MSG_DISCOVERYMODE     1921L

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

#define XML_BUFFER_LEN 32*1024
static int _vXmlBufferMaxLen=0;
static int _vXmlBufferOffset=0;
static char *_pXmlBuffer=NULL;
static size_t myrecv(struct soap* pSoap, char *pData, size_t vLen)
{
   if(_pXmlBuffer==NULL)
   {
      return 0;
   }
   
   if(_vXmlBufferMaxLen - _vXmlBufferOffset < vLen)
   {
      vLen = _vXmlBufferMaxLen - _vXmlBufferOffset;
   }
 
   memcpy(pData, _pXmlBuffer+_vXmlBufferOffset, vLen);
   _vXmlBufferOffset += vLen;
   return vLen;     
}

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
   int i=0;
   int msocket_cli1 = 0, msocket_cli2 = 0;
   int msocket_srv = 0;
   int vReciveLen=0;
   
   char *pAddress=NULL, *pAddressWifi=NULL;
   struct soap* pSoap = NULL;
   
   initMyIpString();
   
   pAddress = getMyIpString(INTERFACE_NAME_1);
   pAddressWifi = getMyIpString(INTERFACE_NAME_2);
   
   InitMyRandom(pAddress);   
   if(pAddress)
   {
      msocket_cli1 = CreateMulticastClient(pAddress, MULTICAST_PORT);
   }
   if(pAddressWifi)
   {      
      msocket_cli2 = CreateMulticastClient(pAddressWifi, MULTICAST_PORT);
   }   
   
   msocket_srv = CreateMulticastServer(MULTICAST_ADDR, MULTICAST_PORT);
   pSoap = soap_new1(SOAP_IO_UDP);
   
   // send 3 times to avoid packet loss 
   for(i=0;i<3;i++)
   {
      if(msocket_cli1>0)
         SendHello(msocket_cli1, pAddress); 
      if(msocket_cli2>0)      
         SendHello(msocket_cli2, pAddressWifi); 
   }   

   free(pAddress);
   free(pAddressWifi);
   close(msocket_cli1);
   close(msocket_cli2);

   thread_ret=pthread_create( &tptr[thread_no].thread_tid, NULL, (void *) RecvThread, (void*)&thread_no );
   if(thread_ret!=0)
   {
      fprintf (stderr, "Create pthread error!\n");
      exit (1);
   }
   thread_no++;   
            
   while(1)
   {
            
      int vDataLen =0;
      struct sockaddr_in vxSockAddrFrom;
      memset((void*)&vxSockAddrFrom, 0, sizeof(vxSockAddrFrom));      
      
      vDataLen = XML_BUFFER_LEN;
      _vXmlBufferMaxLen = XML_BUFFER_LEN;
      _pXmlBuffer = malloc(_vXmlBufferMaxLen+1);
      memset(_pXmlBuffer, 0, _vXmlBufferMaxLen+1);
      
      // Reference www.cnblogs.com/kissazi2/p/3158603.html
      char cmbuf[1024], iovbuf[XML_BUFFER_LEN];
      struct sockaddr_in localaddr;
      struct iovec iov[1];
      
      localaddr.sin_family = AF_INET;
      localaddr.sin_addr.s_addr = inet_addr(pAddress);
      iov[0].iov_base = _pXmlBuffer;//iovbuf;
      iov[0].iov_len = sizeof(iovbuf);
      
      struct msghdr mh = {
         .msg_name = &localaddr,
         .msg_namelen = sizeof(localaddr),
         .msg_control = cmbuf,
         .msg_controllen = sizeof(cmbuf),
         .msg_iov = iov,
         .msg_iovlen = 1,
      };
      
      vReciveLen = recvmsg(msocket_srv, &mh, 0);
      //DBG("set msg_namelen=%d msg_iovlen=%d msg_controllen=%d, vReciveLen=%d\n", mh.msg_namelen, mh.msg_iovlen, mh.msg_controllen,vReciveLen);
                 
      struct cmsghdr *cmsg = NULL;      
      struct in_pktinfo *pi = NULL;           
      for(cmsg = CMSG_FIRSTHDR(&mh) ;
          cmsg != NULL;
          cmsg = CMSG_NXTHDR(&mh, cmsg))
      {
         if(cmsg->cmsg_level != IPPROTO_IP || cmsg->cmsg_type != IP_PKTINFO)
         {
            printf("cmsg->cmsg_level=%d\n", cmsg->cmsg_level);
            continue;
         }        
         else if(cmsg->cmsg_type == IP_PKTINFO)
         {
            pi = (struct in_pktinfo *)CMSG_DATA(cmsg); 
            if(pi)
            {
               char *pTmp, pSrc[32]={0}, pDst[32]={0};
                  
               // inet_ntoa() use a global buffer to store the string,
               // so we need to copy the value before we invoke inet_ntoa() next time        
               pTmp = inet_ntoa(pi->ipi_addr);
               if(pTmp)
                  memcpy(pDst, pTmp, strlen(pTmp));

               pTmp = inet_ntoa(localaddr.sin_addr);
               if(pTmp)
                  memcpy(pSrc, pTmp, strlen(pTmp));
                 
               DBG("%s %s :%d nIndex=%d pSrc=%s:%d, pDst=%s\n",__FILE__,__func__, __LINE__, pi->ipi_ifindex, pSrc, ntohs(localaddr.sin_port), pDst);                  
               DBG("%s %s :%d localaddr=%s port=%d\n",__FILE__,__func__, __LINE__, inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));   
                                             
               if(strncmp(pDst, MULTICAST_ADDR, strlen(MULTICAST_ADDR))==0)
               {
                  // The receive packet is send to host's multicast address
                  SetMulticastFlag(1);
               }
               else
               {
                  // The receive packet is send to host's unicast address
                  SetMulticastFlag(0);
               }                  
                                 
               pSoap->peer.sin_addr.s_addr = inet_addr(pSrc);
               pSoap->peer.sin_port = localaddr.sin_port;//htons(MULTICAST_PORT);
               pSoap->sendsk = pi->ipi_ifindex;
               //pSoap->sendsk = msocket_srv;
                              
            }    
            break;
         }
         else if(cmsg->cmsg_type == IPPROTO_IP)
         {         
            DBG("%s %s :%d Good Job\n",__FILE__,__func__, __LINE__);   
         }
      }

      pSoap->frecv = myrecv;
    
      soap_serve(pSoap);
      DBG("%s %s :%d peer addr=%s, port=%d, error=%d\n",__FILE__,__func__, __LINE__, inet_ntoa(pSoap->peer.sin_addr), ntohs(pSoap->peer.sin_port),  pSoap->error);   
      if(pSoap->header)
      {
         DBG("%s %s :%d Action = %s\n",__FILE__,__func__, __LINE__, pSoap->header->wsa5__Action);
         
#ifdef __APPLE__         
         // This program is test on MAC, so I use "fault" to stop it
         // When run on LINUX OS, it should not stop when receive "fault"
         if(strstr(pSoap->header->wsa5__Action,"fault")!=NULL)
         {
            // exit the loop when receive fault
            // TODO: mark me
            break;
         }
#endif         
      }
      soap_destroy(pSoap);
      soap_end(pSoap);

      _vXmlBufferMaxLen=0;
      _vXmlBufferOffset=0;
      free(_pXmlBuffer);
      _pXmlBuffer=NULL;      
   }
   
   soap_done(pSoap);
   soap_free(pSoap);
   close(msocket_srv);
   return 1;
}

int _client(int argc, char **argv)
{
   int msocket_cli = 0, msocket_cli2= 0;
   char *pAddress=NULL, *pAddressWifi=NULL;
   int vDataLen = XML_BUFFER_LEN;
   char pDataBuf[XML_BUFFER_LEN];
   
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
         SendHello(msocket_cli, pAddress); 
      }
      else if(strcmp(argv[1],"2")==0)
      {
         int vReciveLen=0;
         struct sockaddr_in vxSockAddr;
         SOAP_SOCKLEN_T vSockLen = (SOAP_SOCKLEN_T)sizeof(vxSockAddr);
         memset((void*)&vxSockAddr, 0, sizeof(vxSockAddr));

         SendProbe(msocket_cli); 
         
         DBG("Waiting Response\n");
         
         vDataLen = sizeof(pDataBuf);
         vReciveLen = recvfrom(msocket_cli, pDataBuf, (SOAP_WINSOCKINT)vDataLen, MSG_WAITALL, (struct sockaddr*)&vxSockAddr, &vSockLen);
         DBG("got Response, vReciveLen=%d, \n", vReciveLen);
         DBG("msocket_cli=%d, rsp s_addr=%s, rsp sin_port=%d\n", msocket_cli, inet_ntoa(vxSockAddr.sin_addr), htons(vxSockAddr.sin_port));
         DBG("Buf=%s\n", pDataBuf);
      }               
      else if(strcmp(argv[1],"3")==0)
      {
         int vErr=0;
         struct sockaddr_in vxSockAddr;
         SOAP_SOCKLEN_T vSockLen = (SOAP_SOCKLEN_T)sizeof(vxSockAddr);
         memset((void*)&vxSockAddr, 0, sizeof(vxSockAddr));

         SendResolve(msocket_cli); 
         
         DBG("Waiting Response\n");
         
         vDataLen = sizeof(pDataBuf);
         vErr = recvfrom(msocket_cli, pDataBuf, (SOAP_WINSOCKINT)vDataLen, MSG_WAITALL, (struct sockaddr*)&vxSockAddr, &vSockLen);
         DBG( "got Response, vErr=%d, \n", vErr);
         DBG("msocket_cli=%d, rsp s_addr=%s, rsp sin_port=%d\n", msocket_cli, inet_ntoa(vxSockAddr.sin_addr), htons(vxSockAddr.sin_port));
         DBG("Buf=%s\n", pDataBuf);               
      }               
      else if(strcmp(argv[1],"4")==0)
      {
         SendBye(msocket_cli, pAddress); 
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
         #if 1
         int usocket_cli = 0;
         struct sockaddr_in vSockAddr;
               
         memset((char *) &vSockAddr, 0, sizeof(vSockAddr));
         vSockAddr.sin_family = AF_INET;
         printf("\n\n pAddress=%s, pAddressWifi=%s",pAddress, pAddressWifi);
         vSockAddr.sin_addr.s_addr = inet_addr(pAddress);
         
         vSockAddr.sin_port = htons(MULTICAST_PORT);
         usocket_cli = CreateUnicastClient(&vSockAddr, MULTICAST_PORT);         
         SendFault(usocket_cli, &vSockAddr, NULL); 
         
         #else
         SendFault(msocket_cli, &gMSockAddr, NULL);
         #endif
      }                                                                  
   }
   else
   {
      SendHello(msocket_cli, pAddress); 
   }
   close(msocket_cli);
   
   free(pAddress);
   free(pAddressWifi);
   
   return 1;
}


void RecvThread(void* data)
{
   int i;
   int msqid;
   int msocket_cli = 0, msocket_cli2 = 0;
   char *pAddress=NULL, *pAddressWifi=NULL;    
   pthread_detach(pthread_self());
                  
   DBG("RecvThread start....\n");
   if((msqid = msgget(ONVIF_DIS_MSG_KEY, PERMS | IPC_CREAT)) >= 0)
   {
      native_msg_buf  recvmsg ;
      while(1)
      {                    
         DBG("start recv msg .. \n");
         if(msgrcv(msqid, &recvmsg, 4, 0, 0) > 0)
         {
            initMyIpString();   
            pAddress = getMyIpString(INTERFACE_NAME_1);
            pAddressWifi = getMyIpString(INTERFACE_NAME_2);
            
            if(pAddress)
               msocket_cli = CreateMulticastClient(pAddress, MULTICAST_PORT);
            if(pAddressWifi)
               msocket_cli2 = CreateMulticastClient(pAddressWifi, MULTICAST_PORT);
               
            if(recvmsg.mtype == ONVIF_MSG_UPDATE_SCOPES)
            {
               nativeIncreaseMetadataVersion();

               // send 3 times to avoid packet loss 
               for(i=0;i<3;i++)
               {
                  if(msocket_cli>0)
                     SendHello(msocket_cli, pAddress); 
                  if(msocket_cli2>0)      
                     SendHello(msocket_cli2, pAddressWifi); 
               }               
            } 
            else if(recvmsg.mtype == ONVIF_MSG_REBOOT)
            {
               for (i=0;i<2;i++)
               {
                  // may sleep for a while, so that http response can send before Bye
                  sleep(1);
                  for(i=0;i<3;i++)
                  {
                     if(msocket_cli>0)
                        SendBye(msocket_cli, pAddress); 
                     if(msocket_cli2>0)      
                        SendBye(msocket_cli2, pAddressWifi); 
                  }                                 
               }
            } 
            else if(recvmsg.mtype == ONVIF_MSG_DISCOVERYMODE)
            {
               //initDiscoveryMode();
               DBG("recvmsg.mtext[0]=%c\n",recvmsg.mtext[0]);
               nativeChangeDiscoveryMode(recvmsg.mtext[0]);
               for(i=0;i<3;i++)
               {
                  if(msocket_cli>0)
                     SendHello(msocket_cli, pAddress); 
                  if(msocket_cli2>0)      
                     SendHello(msocket_cli2, pAddressWifi); 
               }  
            }
            close(msocket_cli);
            close(msocket_cli2);
            free(pAddress);
            free(pAddressWifi);
            usleep(500000);
         }
         usleep(500000);
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
