#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifdef __APPLE__
    #include <sys/socket.h>
    #include <sys/sysctl.h>
    #include <net/if.h>
    #include <net/if_dl.h>
#else
    #include <ifaddrs.h>
#endif


#include "util.h"
#include "soapH.h"
#include "porting.h"

static struct soap *_gpSoap=NULL;
char gpLocalAddr[NET_MAX_INTERFACE][32]={{0}};
char gpMacAddr[NET_MAX_INTERFACE][32]={{0}};
#define LOCAL_ADDR gpLocalAddr//"192.168.2.102"

static int _gIsMulticast = 0;

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

void SetMulticastFlag(int bFlag)
{
   _gIsMulticast = bFlag;
}

int GetMulticastFlag()
{
   return _gIsMulticast;
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

void MyFree(void *ptr)
{  
   if(_gpSoap!=NULL)
      soap_dealloc(_gpSoap, ptr);
   else
      free(ptr);
}

char * CopyString(char *pSrc)
{
   int vLen = 0;
   char *pDst = NULL;
   
   if(!pSrc) return NULL;
   
   vLen = strlen(pSrc);
   pDst = MyMalloc(vLen+1); 
   memcpy(pDst, pSrc, vLen);
   
   return pDst;
}

char * MySoapCopyString(struct soap *pSoap, char *pSrc)
{
   int vLen = 0;
   char *pDst = NULL;
   
   if(!pSoap) return NULL;
   if(!pSrc) return NULL;
   
   vLen = strlen(pSrc);
   pDst = soap_malloc(pSoap, vLen+1);
   memset(pDst, 0, vLen+1);
   memcpy(pDst, pSrc, vLen);
   
   return pDst;
}

char * getMyMacAddress(void)
{
   return CopyString(gpMacAddr[0]);
}

// Utilities of Network
char * getMyIpString(char *pIfName)
{
   int i = 0;
   if(pIfName==NULL)
   return NULL;
   
   for(i=0;i<NET_MAX_INTERFACE;i++)
   {
      if(strncmp("en1", pIfName, 3)==0)
      {
         return CopyString(gpLocalAddr[1]);
      } 
   }
   return CopyString(gpLocalAddr[0]);
}


char * initMyIpString(void)
{
   int vInterfaceCount=0;
   char pInterface[128]={0};
   struct ifaddrs * ifAddrStruct=NULL;
   struct ifaddrs * ifa=NULL;
   void * tmpAddrPtr=NULL;
   
   getifaddrs(&ifAddrStruct);
   
   for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
   {
      if (ifa ->ifa_addr->sa_family==AF_INET) 
      {   
         // check it is IP4
         // is a valid IP4 Address
         char addressBuffer[INET_ADDRSTRLEN];
         tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
         
         inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
         //DBG("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
         if(strncmp(ifa->ifa_name, INTERFACE_NAME_1, 2)==0)
         {
            // Note: you may set local address for different interface. For example:eth0, eth1
            memcpy(gpLocalAddr[vInterfaceCount], addressBuffer, strlen(addressBuffer));
            memset(pInterface, 0 ,128);
            memcpy(pInterface, ifa->ifa_name, strlen(ifa->ifa_name));
            
            #ifdef __APPLE__
            // Reference https://gist.github.com/Coeur/1409855
            {
                int                 mgmtInfoBase[6];
                char                *msgBuffer = NULL;
                char                *errorFlag = NULL;
                size_t              length;

                // Setup the management Information Base (mib)
                mgmtInfoBase[0] = CTL_NET;        // Request network subsystem
                mgmtInfoBase[1] = AF_ROUTE;       // Routing table info
                mgmtInfoBase[2] = 0;
                mgmtInfoBase[3] = AF_LINK;        // Request link layer information
                mgmtInfoBase[4] = NET_RT_IFLIST;  // Request all configured interfaces


                // The interface name should be decided runtime
                int sock;
                struct ifreq ifr;

                sock = socket(AF_INET, SOCK_DGRAM, 0);
                ifr.ifr_addr.sa_family = AF_INET;

                strncpy(ifr.ifr_name, ifa->ifa_name, strlen(ifa->ifa_name));
                close(sock);
                
                // With all configured interfaces requested, get handle index
                //if ((mgmtInfoBase[5] = if_nametoindex("en0")) == 0)
                if ((mgmtInfoBase[5] = if_nametoindex(ifr.ifr_name)) == 0)
                    errorFlag = "if_nametoindex failure";
                // Get the size of the data available (store in len)
                else if (sysctl(mgmtInfoBase, 6, NULL, &length, NULL, 0) < 0)
                    errorFlag = "sysctl mgmtInfoBase failure";
                // Alloc memory based on above call
                else if ((msgBuffer = malloc(length)) == NULL)
                    errorFlag = "buffer allocation failure";
                // Get system information, store in buffer
                else if (sysctl(mgmtInfoBase, 6, msgBuffer, &length, NULL, 0) < 0) {
                    free(msgBuffer);
                    errorFlag = "sysctl msgBuffer failure";
                } else {
                    // Map msgbuffer to interface message structure
                    struct if_msghdr *interfaceMsgStruct = (struct if_msghdr *) msgBuffer;

                    // Map to link-level socket structure
                    struct sockaddr_dl *socketStruct = (struct sockaddr_dl *) (interfaceMsgStruct + 1);

                    // Copy link layer address data in socket structure to an array
                    unsigned char macAddress[6]= {0};
                    memcpy(&macAddress, socketStruct->sdl_data + socketStruct->sdl_nlen, 6);

                    // Read from char array into a string object, into traditional Mac address format
                    sprintf(gpMacAddr[vInterfaceCount], "%.2x%.2x%.2x%.2x%.2x%.2x",
                            (unsigned char)macAddress[0],
                            (unsigned char)macAddress[1],
                            (unsigned char)macAddress[2],
                            (unsigned char)macAddress[3],
                            (unsigned char)macAddress[4],
                            (unsigned char)macAddress[5]);
                    DBG("MAC %s\n", gpMacAddr[vInterfaceCount]);

                    // Release the buffer memory
                    free(msgBuffer);
                }

                if(errorFlag) printf("err = %s\n", errorFlag);
            }
            
            #else
     
            {
               // For linux system
               int sock;
               struct ifreq ifr;
               
               sock = socket(AF_INET, SOCK_DGRAM, 0);
               ifr.ifr_addr.sa_family = AF_INET;
               
               strncpy(ifr.ifr_name, pInterface, IFNAMSIZ-1);
               
               ioctl(sock, SIOCGIFHWADDR, &ifr);
               
               close(sock);
               
               sprintf(gpMacAddr[vInterfaceCount], "%.2x%.2x%.2x%.2x%.2x%.2x", 
               (unsigned char)ifr.ifr_hwaddr.sa_data[0],
               (unsigned char)ifr.ifr_hwaddr.sa_data[1],
               (unsigned char)ifr.ifr_hwaddr.sa_data[2],
               (unsigned char)ifr.ifr_hwaddr.sa_data[3],
               (unsigned char)ifr.ifr_hwaddr.sa_data[4],
               (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
               //DBG("MAC %s\n", gpMacAddr[vInterfaceCount]); 
            }
            #endif
            vInterfaceCount++;
         } 
      } 
      else if (ifa->ifa_addr->sa_family==AF_INET6) 
      {   
         // check it is IP6
         // is a valid IP6 Address
         tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
         char addressBuffer[INET6_ADDRSTRLEN];
         inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
         //DBG("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
      }         
   }
   
   
   DBG("gpLocalAddr is set to %s, MAC is %s\n\n", gpLocalAddr[0], gpMacAddr[0]);    
   if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
   return gpLocalAddr[0];
}

int CreateUnicastClient(struct sockaddr_in *pSockAddr, int port)
{
   // http://www.tenouk.com/Module41c.html
   int sd=-1;
   
   struct timeval timeout;
   timeout.tv_sec  = 10;
   timeout.tv_usec = 0;
   
   sd = socket(AF_INET, SOCK_DGRAM, 0);
   if(sd < 0)
   {
      perror("Opening datagram socket error");
      exit(1);
   }
   else
      DBG("Opening the datagram socket %d...OK.\n", sd);
   
   if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
   {
      DBG("setsockopt...error.\n");
   }
   
#if 0
   int reuse=1;
   struct sockaddr_in localSock;   
   if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
   {
      perror("Setting SO_REUSEADDR error");
      close(sd);
      exit(1);
   }
   else
      DBG("Setting SO_REUSEADDR...OK.\n");
         
   // The host may have many interface
   // If needed, we may create the socket to bind the interface that data was sent.
   localSock.sin_family = AF_INET;
   localSock.sin_port = htons(MULTICAST_PORT);
   //localSock.sin_addr.s_addr = htonl(INADDR_ANY);
   localSock.sin_addr.s_addr = inet_addr(gpLocalAddr[0]);
   if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
   {
      perror("Binding datagram socket error");
      close(sd);
      exit(1);
   }
   else
      DBG("Binding port:%d socket...OK.\n",port);
#endif
   
   return sd;
}


int CreateMulticastClient(char *pAddress, int port)
{
   // http://www.tenouk.com/Module41c.html
   struct in_addr localInterface;
   int sd=-1;
   
   struct timeval timeout;
   timeout.tv_sec  = 10;
   timeout.tv_usec = 0;
   
   if(!pAddress)
      return -1;
   if(strlen(pAddress)==0)
      return -1;
   
   sd = socket(AF_INET, SOCK_DGRAM, 0);
   if(sd < 0)
   {
      perror("Opening datagram socket error");
      exit(1);
   }
   else
      DBG("Opening multicast client socket %d for ip %s...OK.\n", sd, pAddress);
   
   if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
   {
      DBG("setsockopt...error.\n");
   }
         
   memset((char *) &gMSockAddr, 0, sizeof(gMSockAddr));
   gMSockAddr.sin_family = AF_INET;
   gMSockAddr.sin_addr.s_addr = inet_addr(MULTICAST_ADDR);
   gMSockAddr.sin_port = htons(port);
   
   localInterface.s_addr = inet_addr(pAddress);
   
   if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface.s_addr, sizeof(localInterface)) < 0)
   {
      perror("Setting local interface error");
      exit(1);
   }
   else
      DBG("Setting the local interface...OK\n");    
   
   return sd;
}

int CreateMulticastServer(char *pAddress, int port)
{
   struct ip_mreq group;
   struct sockaddr_in localSock;
   int i, sd;
   int reuse = 1, loop = 0;
   
   sd = socket(AF_INET, SOCK_DGRAM, 0);

   if(sd < 0)
   {
      perror("Opening datagram socket error");
      exit(1);
   }
   else
      DBG("Opening multicast server socket %d for ip %s...OK.\n",sd, pAddress);
   

   if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
   {
      perror("Setting SO_REUSEADDR error");
      close(sd);
      exit(1);
   }
   else
      DBG("Setting SO_REUSEADDR...OK.\n");


    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) < 0)
    {
        perror("Setting IP_MULTICAST_LOOP error");
        close(sd);
        exit(1);
    }
    else
        DBG("Disable loopback...OK.\n");
    
   memset((char *) &localSock, 0, sizeof(localSock));
   localSock.sin_family = AF_INET;
   localSock.sin_port = htons(port);
   localSock.sin_addr.s_addr = INADDR_ANY;
   
   int opt = 1;
   if(setsockopt(sd, IPPROTO_IP, IP_PKTINFO, &opt, sizeof(opt)) < 0)
      printf("set IP_PKTINFO error\n");
       
   for(i=0;i<NET_MAX_INTERFACE;i++)
   {
      group.imr_multiaddr.s_addr = inet_addr(pAddress);
      if(strlen(gpLocalAddr[i])!=0)
      {
         group.imr_interface.s_addr = inet_addr(gpLocalAddr[i]); 
         // In MAC, if we set INADDR_ANY, it will set only the 1st network interface
         // group.imr_interface.s_addr = INADDR_ANY;
         if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
         {
            perror("setsockopt IP_ADD_MEMBERSHIP error");
            // This error may happened if some process had already do IP_ADD_MEMBERSHIP
            // This error can be omited
            //close(sd);
            //exit(1);
         }
         else
         {
            DBG("setsockopt IP_ADD_MEMBERSHIP for %s ...OK.\n", gpLocalAddr[i]);
         }
      }
   }
               
   if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
   {
      perror("Binding datagram socket error");
      close(sd);
      exit(1);
   }
   else
      DBG("Binding port:%d socket...OK.\n",port);
   
   return sd;
}


int CreateUnicastServer(char *pAddress, int vPort)
{
   int sd;
   struct sockaddr_in localSock;   
   
   if(!pAddress)
      return -1;
   if(strlen(pAddress)==0)
      return -1;
         
   sd = socket(AF_INET, SOCK_DGRAM, 0);
   if(sd < 0)
   {
      perror("Opening datagram socket error");
      exit(1);
   }
   else
      DBG("Opening unicast server socket %d for ip %s...OK.\n",sd, pAddress);
   
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
   localSock.sin_port = htons(vPort);
   localSock.sin_addr.s_addr = inet_addr(pAddress);
   
   if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
   {
      perror("Binding datagram socket error");
      close(sd);
      exit(1);
   }
   else
      DBG("Binding datagram socket...OK.\n");
   
   return sd;
}


// matchBy function
int match_rfc3986(char *pItem)
{ 
   char *pSystemScopes = nativeGetScopesItem();
   if(strstr(pSystemScopes,pItem))
   {
      free(pSystemScopes);
      return 1;
   }
   else 
   {
      free(pSystemScopes);
      return 0;
   }
}

int match_uuid(char *pItem)
{ 
   char *pSystemScopes = nativeGetScopesItem();
   if(strstr(pSystemScopes,pItem))
   {
      free(pSystemScopes);
      return 1;
   }
   else 
   {
      free(pSystemScopes);
      return 0;
   }
}


int match_ldap(char *pItem)
{ 
   char *pSystemScopes = nativeGetScopesItem();
   if(strstr(pSystemScopes,pItem))
   {
      free(pSystemScopes);
      return 1;
   }
   else 
   {
      free(pSystemScopes);
      return 0;
   }
}


int match_strcmp0(char *pItem)
{ 
   char *pSystemScopes = nativeGetScopesItem();
   if(strstr(pSystemScopes,pItem))
   {
      free(pSystemScopes);
      return 1;
   }
   else 
   {
      free(pSystemScopes);
      return 0;
   }
}


int match_none(char *pItem)
{ 
   char *pSystemScopes = nativeGetScopesItem();
   if(strstr(pSystemScopes,pItem))
   {
      free(pSystemScopes);
      return 1;
   }
   else 
   {
      free(pSystemScopes);
      return 0;
   }
}

              
              
// Random function
void InitMyRandom(char *myipaddr)
{
   unsigned int ourAddress;
   struct timeval timeNow;
   
   ourAddress = ntohl(inet_addr(myipaddr));
   gettimeofday(&timeNow, NULL);
   
   unsigned int seed = ourAddress^timeNow.tv_sec^timeNow.tv_usec;
     
   srandom(seed);
}

long our_random() 
{
   return random();
}

unsigned int our_random16()
{
   long random1 = our_random();
   return (unsigned int)(random1&0xffff);
}


unsigned int our_random32() 
{  
   long random1 = our_random();
   long random2 = our_random();
   
   return (unsigned int)((random2<<31) | random1);
}
              
void UuidGen(char *uuidbuf)
{
   sprintf(uuidbuf, "%08x-%04x-%04x-%04x-%08x%04x",our_random32(), our_random16(),our_random16(),our_random16(),our_random32(), our_random16());    
}     


void noprintf(char *format, ...)
{
   ;
}        
