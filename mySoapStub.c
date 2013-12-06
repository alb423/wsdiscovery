/*
   If you find any bug, please let me know. Mail to alb423@gmail.com
*/

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
#include "porting.h"
#include "wsdd.nsmap" //  SOAP_NMAC struct Namespace namespaces[]

// Send Multicast Packet (Hello and Bye)
int SendHello(int socket, char *pXAddrsIn)
{
   int vErr = 0;
   struct __wsdd__Hello *pWsdd__Hello = NULL;
   struct wsdd__HelloType *pWsdd__HelloType = NULL;   
   struct soap *pSoap = NULL;
   
   char *pAction=NULL, *pMessageID=NULL, *pTo=NULL;
   char *pEndpointAddress=NULL, *pTypes=NULL, *pItem=NULL, *pXAddrs=NULL, *pMatchBy=NULL;
   
   if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
      return 0;
   
   // Get evnironment variable
#if WSDISCOVERY_SPEC_VER == WSDISCOVERY_SPEC_200901   
   pAction  = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Hello");
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
#else   
   pAction  = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello");
   pMatchBy = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986");
#endif   
   pMessageID = nativeGetMessageId();
   pTo = nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   pXAddrs = nativeGetXAddrs(pXAddrsIn);
      
   pSoap = soap_new1(SOAP_IO_UDP);      
   pSoap->fsend = mysend;
   
   // Build SOAP Header
   soap_header(pSoap);
   pSoap->header->wsa5__Action = MySoapCopyString(pSoap, pAction);
   pSoap->header->wsa5__MessageID = MySoapCopyString(pSoap, pMessageID);
   pSoap->header->wsa5__To = MySoapCopyString(pSoap, pTo);
   
   pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
   soap_default_wsdd__AppSequenceType(pSoap, pSoap->header->wsdd__AppSequence);
   pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
   pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
   pSoap->header->wsdd__AppSequence->SequenceId = NULL;

   // Build Hello Message
   pWsdd__Hello = (struct __wsdd__Hello *) soap_malloc(pSoap,sizeof(struct __wsdd__Hello));
   soap_default___wsdd__Hello(pSoap, pWsdd__Hello);
   pWsdd__HelloType = (struct wsdd__HelloType *) soap_malloc(pSoap, sizeof(struct wsdd__HelloType));
   soap_default_wsdd__HelloType(pSoap, pWsdd__HelloType);
      
   //pSoap->encodingStyle = NULL;
   pWsdd__Hello->wsdd__Hello = pWsdd__HelloType;   
   
   pWsdd__HelloType->wsa5__EndpointReference.Address = MySoapCopyString(pSoap, pEndpointAddress);
   pWsdd__HelloType->Types = MySoapCopyString(pSoap, pTypes);
   pWsdd__HelloType->Scopes = (struct wsdd__ScopesType *)soap_malloc(pSoap, sizeof(struct wsdd__ScopesType));
   pWsdd__HelloType->Scopes->MatchBy = MySoapCopyString(pSoap, pMatchBy);
   pWsdd__HelloType->Scopes->__item = MySoapCopyString(pSoap, pItem);
   pWsdd__HelloType->XAddrs = MySoapCopyString(pSoap, pXAddrs);
   pWsdd__HelloType->MetadataVersion = nativeGetMetadataVersion();

            
   soap_serializeheader(pSoap);
   soap_response(pSoap, SOAP_OK);
   soap_envelope_begin_out(pSoap);
   soap_putheader(pSoap);
   soap_body_begin_out(pSoap);
   vErr = soap_put___wsdd__Hello(pSoap, pWsdd__Hello, "-wsdd:Hello", "wsdd:HelloType");
   soap_body_end_out(pSoap);
   soap_envelope_end_out(pSoap);
   soap_destroy(pSoap);
   soap_end(pSoap);
   
   soap_free(pSoap);
   
   
   free(pAction);
   free(pMessageID);
   free(pTo);
   free(pEndpointAddress);
   free(pTypes);
   free(pItem);
   free(pXAddrs);
   free(pMatchBy);

   // Chapter 3.1.3
   // Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
   // This timer MUST be set to a random value between 0 and APP_MAX_DELAY
   usleep( (random()%APP_MAX_DELAY) ); 
   
   char *pBuffer = getXmlBufferData();
   int vBufLen = strlen(pBuffer);
   //DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
   
   if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
   {
      perror("Sending datagram message error");
   }
   else
     DBG("Sending SendHello message...OK\n");      
     
   clearXmlBuffer();
     
   return SOAP_OK;
}

int SendBye(int socket, char *pXAddrsIn)
{
   int vErr = 0;
   struct __wsdd__Bye *pWsdd__Bye = NULL;
   struct wsdd__ByeType *pWsdd__ByeType = NULL;
   struct soap *pSoap = NULL;

   char *pAction=NULL, *pMessageID=NULL, *pTo=NULL;
   char *pEndpointAddress=NULL, *pTypes=NULL, *pItem=NULL, *pXAddrs=NULL, *pMatchBy=NULL;
   
   if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
      return 0;
   
   // Get evnironment variable
#if WSDISCOVERY_SPEC_VER == WSDISCOVERY_SPEC_200901   
   pAction =  CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Bye");
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
#else   
   pAction =  CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/Bye");
   pMatchBy = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986");
#endif   
   pMessageID = nativeGetMessageId();
   pTo =nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   pXAddrs = nativeGetXAddrs(pXAddrsIn);
   
         
   pSoap = soap_new1(SOAP_IO_UDP);      
   pSoap->fsend = mysend;
   
   // Build SOAP Header
   soap_header(pSoap);
   pSoap->header->wsa5__Action = MySoapCopyString(pSoap, pAction);
   pSoap->header->wsa5__MessageID = MySoapCopyString(pSoap, pMessageID);
   pSoap->header->wsa5__To = MySoapCopyString(pSoap, pTo);
   pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
   soap_default_wsdd__AppSequenceType(pSoap, pSoap->header->wsdd__AppSequence);
   pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
   pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
   pSoap->header->wsdd__AppSequence->SequenceId = NULL;
      

   // Build Bye Message
   pWsdd__Bye = (struct __wsdd__Bye *) soap_malloc(pSoap,sizeof(struct __wsdd__Bye));
   soap_default___wsdd__Bye(pSoap, pWsdd__Bye);
   pWsdd__ByeType = (struct wsdd__ByeType *) soap_malloc(pSoap, sizeof(struct wsdd__ByeType));
   soap_default_wsdd__ByeType(pSoap, pWsdd__ByeType);      
   //pSoap->encodingStyle = NULL;
      
   pWsdd__Bye->wsdd__Bye = pWsdd__ByeType;   
   pWsdd__ByeType->wsa5__EndpointReference.Address = MySoapCopyString(pSoap, pEndpointAddress);
   pWsdd__ByeType->Types = MySoapCopyString(pSoap, pTypes);
   pWsdd__ByeType->Scopes = (struct wsdd__ScopesType *)soap_malloc(pSoap, sizeof(struct wsdd__ScopesType));
   pWsdd__ByeType->Scopes->MatchBy = MySoapCopyString(pSoap, pMatchBy); 
   pWsdd__ByeType->Scopes->__item = MySoapCopyString(pSoap, pItem);
   pWsdd__ByeType->XAddrs = MySoapCopyString(pSoap, pXAddrs);
   pWsdd__ByeType->MetadataVersion = MyMalloc(sizeof(int));
   *pWsdd__ByeType->MetadataVersion = nativeGetMetadataVersion();
               
   
   soap_serializeheader(pSoap);

   soap_response(pSoap, SOAP_OK);
   soap_envelope_begin_out(pSoap);
   soap_putheader(pSoap);
   soap_body_begin_out(pSoap);
   vErr = soap_put___wsdd__Bye(pSoap, pWsdd__Bye, "-wsdd:Bye", "wsdd:ByeType");
   soap_body_end_out(pSoap);
   soap_envelope_end_out(pSoap);
     
   soap_destroy(pSoap);
   soap_end(pSoap);
   soap_free(pSoap);
   
   free(pAction);
   free(pMessageID);
   free(pTo);
   free(pEndpointAddress);
   free(pTypes);
   free(pItem);
   free(pXAddrs);
   free(pMatchBy);

   // Chapter 3.1.3
   // Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
   // This timer MUST be set to a random value between 0 and APP_MAX_DELAY
   usleep( (random()%APP_MAX_DELAY) ); 
   
   char *pBuffer = getXmlBufferData();
   int vBufLen = strlen(pBuffer);   
   //DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
   
   if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
   {
      perror("Sending datagram message error");
   }
   else
     DBG("Sending SendBye message...OK\n");   

   clearXmlBuffer();        
        
   return SOAP_OK;
}

int SendProbe(int socket)
{
   int vErr = 0;
   struct __wsdd__Probe *pWsdd__Probe = NULL;
   struct wsdd__ProbeType *pWsdd__ProbeType = NULL;   
   struct soap *pSoap=NULL;
   char *pAction=NULL, *pMessageID=NULL, *pTo=NULL;
   char *pEndpointAddress=NULL, *pTypes=NULL, *pItem=NULL, *pMatchBy=NULL;
            
   if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
      return 0;
      
#if WSDISCOVERY_SPEC_VER == WSDISCOVERY_SPEC_200901      
   pAction  = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Probe");
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
#else   
   pAction  = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe");
   pMatchBy = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986");
#endif   
   pMessageID = nativeGetMessageId();
   pTo = nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();

   pSoap = soap_new1(SOAP_IO_UDP);      
   pSoap->fsend = mysend;
   
   // Build SOAP Header
   soap_header(pSoap);
   pSoap->header->wsa5__Action = MySoapCopyString(pSoap, pAction);
   pSoap->header->wsa5__MessageID = MySoapCopyString(pSoap, pMessageID);
   pSoap->header->wsa5__To = MySoapCopyString(pSoap, pTo);
   pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
   soap_default_wsdd__AppSequenceType(pSoap, pSoap->header->wsdd__AppSequence);
   pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
   pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
   pSoap->header->wsdd__AppSequence->SequenceId = NULL;
   

   // Build Probe Message
   pWsdd__Probe = (struct __wsdd__Probe *)soap_malloc(pSoap, sizeof(struct __wsdd__Probe));
   soap_default___wsdd__Probe(pSoap, pWsdd__Probe);
   pWsdd__ProbeType = (struct wsdd__ProbeType *)soap_malloc(pSoap,sizeof(struct wsdd__ProbeType));
   soap_default_wsdd__ProbeType(pSoap, pWsdd__ProbeType); 
   
   pSoap->encodingStyle = NULL;
   
   pWsdd__Probe->wsdd__Probe = pWsdd__ProbeType;   
   pWsdd__ProbeType->Types = MySoapCopyString(pSoap, pTypes);
   pWsdd__ProbeType->Scopes = (struct wsdd__ScopesType *)soap_malloc(pSoap, sizeof(struct wsdd__ScopesType));
   pWsdd__ProbeType->Scopes->MatchBy = MySoapCopyString(pSoap, "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
   #if 1
      pWsdd__ProbeType->Scopes->__item = MySoapCopyString(pSoap, pItem);
   #else
      // Test for invalid scope
      pWsdd__ProbeType->Scopes->__item = MySoapCopyString(pSoap, "undefinedScope");
   #endif
      
   soap_serializeheader(pSoap);

   soap_response(pSoap, SOAP_OK);
   soap_envelope_begin_out(pSoap);
   soap_putheader(pSoap);
   soap_body_begin_out(pSoap);
   vErr = soap_put___wsdd__Probe(pSoap, pWsdd__Probe, "-wsdd:Probe", "wsdd:ProbeType");
   soap_body_end_out(pSoap);
   soap_envelope_end_out(pSoap);
   soap_end_send(pSoap);
   soap_destroy(pSoap);
   soap_end(pSoap);
   soap_free(pSoap);
         
   free(pAction);
   free(pMessageID);
   free(pTo);
   free(pEndpointAddress);
   free(pTypes);
   free(pItem);
   free(pMatchBy);
            
   // Chapter 3.1.3
   // Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
   // This timer MUST be set to a random value between 0 and APP_MAX_DELAY         
   usleep( (random()%APP_MAX_DELAY) ); 
   
   char *pBuffer = getXmlBufferData();
   int vBufLen = strlen(pBuffer);      
   //DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
   
   if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
   {
      perror("Sending datagram message error");
   }
   else
     DBG("Sending SendProbe message...OK\n");   
     
   clearXmlBuffer();     
   return SOAP_OK;
}

int SendResolve(int socket)
{
   int vErr = 0;
   struct __wsdd__Resolve *pWsdd__Resolve = NULL;
   struct wsdd__ResolveType *pWsdd__ResolveType = NULL;   
   struct soap *pSoap=NULL;

   char *pAction=NULL, *pMessageID=NULL, *pTo=NULL;
   char *pEndpointAddress=NULL, *pTypes=NULL, *pItem=NULL, *pMatchBy=NULL;
            
   if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
      return 0;
      
#if WSDISCOVERY_SPEC_VER == WSDISCOVERY_SPEC_200901      
   pAction  = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Resolve");
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
#else   
   pAction  = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/Resolve");
   pMatchBy = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986");
#endif   
   pMessageID = nativeGetMessageId();
   pTo = nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   

   pSoap = soap_new1(SOAP_IO_UDP);      
   pSoap->fsend = mysend;
      
         
   // Build Resolve Message
   pWsdd__Resolve = (struct __wsdd__Resolve *)soap_malloc(pSoap, sizeof(struct __wsdd__Resolve));
   soap_default___wsdd__Resolve(pSoap, pWsdd__Resolve);
   pWsdd__ResolveType = (struct wsdd__ResolveType *)soap_malloc(pSoap,sizeof(struct wsdd__ResolveType));
   soap_default_wsdd__ResolveType(pSoap, pWsdd__ResolveType); 
      
   pSoap->encodingStyle = NULL;
   
   pWsdd__Resolve->wsdd__Resolve = pWsdd__ResolveType;   
   pWsdd__ResolveType->wsa5__EndpointReference.Address = MySoapCopyString(pSoap, pEndpointAddress);
   
   
   soap_serializeheader(pSoap);

   soap_response(pSoap, SOAP_OK);
   soap_envelope_begin_out(pSoap);
   soap_putheader(pSoap);
   soap_body_begin_out(pSoap);
   vErr = soap_put___wsdd__Resolve(pSoap, pWsdd__Resolve, "-wsdd:Resolve", "wsdd:ResolveType");
   soap_body_end_out(pSoap);
   soap_envelope_end_out(pSoap);
   soap_destroy(pSoap);
   soap_end(pSoap);
   soap_free(pSoap);
   
   free(pAction);
   free(pMessageID);
   free(pTo);
   free(pEndpointAddress);
   free(pTypes);
   free(pItem);
   free(pMatchBy);
               
   
   // Chapter 3.1.3
   // Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
   // This timer MUST be set to a random value between 0 and APP_MAX_DELAY   
   usleep( (random()%APP_MAX_DELAY) ); 
   
   char *pBuffer = getXmlBufferData();
   int vBufLen = strlen(pBuffer);   
   DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
   if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
   {
      perror("Sending datagram message error");
   }
   else
     DBG("Sending SendResolve message...OK\n");   
     
   clearXmlBuffer();
     
   return SOAP_OK;
}


int SendProbeMatches(int socket, struct sockaddr_in *pSockAddr_In, char *pSenderMsgId)
{   
   int vErr = 0;
   struct __wsdd__ProbeMatches *pwsdd__ProbeMatches = NULL;
   struct wsdd__ProbeMatchesType *pwsdd__ProbeMatchesType = NULL;
   struct soap *pSoap = NULL;

   char *pAction=NULL, *pMessageID=NULL, *pTo=NULL;
   char *pEndpointAddress=NULL, *pTypes=NULL, *pItem=NULL, *pXAddrs=NULL, *pMatchBy=NULL;
            
   if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
      return 0;
      
#if WSDISCOVERY_SPEC_VER == WSDISCOVERY_SPEC_200901      
   pAction  = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches");
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
#else   
   pAction  = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches");
   pMatchBy = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986");
#endif   
   pMessageID = nativeGetMessageId();
   pTo = nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   pXAddrs = nativeGetXAddrs(inet_ntoa(pSockAddr_In->sin_addr));
   
   
   pSoap = soap_new1(SOAP_IO_UDP);   
   pSoap->fsend = mysend;

   // Build SOAP Header
   soap_header(pSoap);
   pSoap->header->wsa5__Action = MySoapCopyString(pSoap, pAction);
   pSoap->header->wsa5__MessageID = MySoapCopyString(pSoap, pMessageID);
   pSoap->header->wsa5__To = MySoapCopyString(pSoap, "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous");
   pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
   soap_default_wsdd__AppSequenceType(pSoap, pSoap->header->wsdd__AppSequence);
   pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
   pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
   pSoap->header->wsdd__AppSequence->SequenceId = NULL;
   
   // Here cause segmentation fault
   pSoap->header->wsa5__RelatesTo = (struct wsa5__RelatesToType *) soap_malloc(pSoap,sizeof(struct wsa5__RelatesToType));
   soap_default_wsa5__RelatesToType(pSoap, pSoap->header->wsa5__RelatesTo);
   pSoap->header->wsa5__RelatesTo->__item = MySoapCopyString(pSoap, pSenderMsgId);

   // Build ProbeMatch Message
   pwsdd__ProbeMatches = (struct __wsdd__ProbeMatches *) soap_malloc(pSoap,sizeof(struct __wsdd__ProbeMatches));
   soap_default___wsdd__ProbeMatches(pSoap, pwsdd__ProbeMatches);
   pwsdd__ProbeMatchesType = (struct wsdd__ProbeMatchesType *) soap_malloc(pSoap,sizeof(struct wsdd__ProbeMatchesType));
   soap_default_wsdd__ProbeMatchesType(pSoap, pwsdd__ProbeMatchesType);
   //pSoap->encodingStyle = NULL;
         
   pwsdd__ProbeMatches->wsdd__ProbeMatches = pwsdd__ProbeMatchesType;   
   pwsdd__ProbeMatchesType->__sizeProbeMatch = 1;
   
   pwsdd__ProbeMatchesType->ProbeMatch = (struct wsdd__ProbeMatchType *) soap_malloc(pSoap, sizeof(struct wsdd__ProbeMatchType));
   soap_default_wsdd__ProbeMatchType(pSoap, pwsdd__ProbeMatchesType->ProbeMatch);
   
   pwsdd__ProbeMatchesType->ProbeMatch->wsa5__EndpointReference.Address = MySoapCopyString(pSoap, pEndpointAddress);
   pwsdd__ProbeMatchesType->ProbeMatch->Types = MySoapCopyString(pSoap, pTypes);
   
   pwsdd__ProbeMatchesType->ProbeMatch->Scopes = (struct wsdd__ScopesType *)soap_malloc(pSoap, sizeof(struct wsdd__ScopesType));
   soap_default_wsdd__ScopesType(pSoap, pwsdd__ProbeMatchesType->ProbeMatch->Scopes);   
   
   pwsdd__ProbeMatchesType->ProbeMatch->Scopes->__item = MySoapCopyString(pSoap, pItem);
   pwsdd__ProbeMatchesType->ProbeMatch->Scopes->MatchBy = MySoapCopyString(pSoap, pMatchBy);
   pwsdd__ProbeMatchesType->ProbeMatch->XAddrs = MySoapCopyString(pSoap, pXAddrs);
   pwsdd__ProbeMatchesType->ProbeMatch->MetadataVersion = nativeGetMetadataVersion();
         

   soap_serializeheader(pSoap);

   soap_response(pSoap, SOAP_OK); 
   soap_envelope_begin_out(pSoap);
   soap_putheader(pSoap);
   soap_body_begin_out(pSoap);
   vErr = soap_put___wsdd__ProbeMatches(pSoap, pwsdd__ProbeMatches, "-wsdd:ProbeMatches", "wsdd:ProbeMatchType");
   soap_body_end_out(pSoap);
   soap_envelope_end_out(pSoap);
   soap_end_send(pSoap);
   soap_destroy(pSoap);
   soap_end(pSoap);   
   soap_free(pSoap);
   

   free(pAction);
   free(pMessageID);
   free(pTo);
   free(pEndpointAddress);
   free(pTypes);
   free(pItem);
   free(pXAddrs);
   free(pMatchBy);
      
   // Chapter 3.1.3
   // Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
   // This timer MUST be set to a random value between 0 and APP_MAX_DELAY      
   usleep( (random()%APP_MAX_DELAY) ); 
   
   char *pBuffer = getXmlBufferData();
   int vBufLen = strlen(pBuffer);      
   //DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
   
   // Test for maximus sendTo Capability()
   // The length field of UDP is 16 bits, so the max length of a UDP packet should be 2^16=65536
   // But the default limitation of sendto() may be samller.
   // We can use setsockopt() to increase the value. 
#if 0
   
   int bufsize; 
   unsigned int size = sizeof(bufsize);
   getsockopt(socket,SOL_SOCKET, SO_SNDBUF, &bufsize, &size);
   DBG("SO_SNDBUF bufsize=%d\n", bufsize); // 9216
   
   bufsize=65535;
   setsockopt(socket, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
   getsockopt(socket,SOL_SOCKET, SO_SNDBUF, &bufsize, &size);
   DBG("Change SO_SNDBUF bufsize=%d\n", bufsize); // 9216
      
   // test with 60kBuffer
   int i=0, vNum=30; 
   int p60K_vBufLen = vNum*strlen(pBuffer)+1;
   char *p60K_Buffer = malloc(p60K_vBufLen);
   memset(p60K_Buffer, 0, p60K_vBufLen);
   DBG("p60K_Buffer alloc success, len=%d\n", p60K_vBufLen);
      
   // when size is 8921, sendto() ok
   // when size is 10717, sendto() error
   // if we change SO_SNDBUF, than we can send more data
   for(i=0;i<vNum;i++)
   {
      memcpy(p60K_Buffer+(i*vBufLen), pBuffer, vBufLen);
   }
   
   if(sendto(socket, p60K_Buffer, p60K_vBufLen, 0, (struct sockaddr*)pSockAddr_In, sizeof(struct sockaddr_in)) < 0)
   {
      perror("Sending datagram message error");
   }
   else
     DBG("Sending SendProbeMatches message...OK\n");  
     
   free(p60K_Buffer);
  
#else   
   if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)pSockAddr_In, sizeof(struct sockaddr_in)) < 0)
   {
      perror("Sending datagram message error");
   }
   else
     DBG("Sending SendProbeMatches message...OK\n");   
#endif
     
   clearXmlBuffer();
     
   return SOAP_OK;
}

int SendResolveMatches(int socket, struct sockaddr_in *pSockAddr_In, char *pSenderMsgId)
{
   int vErr = 0;
   struct __wsdd__ResolveMatches *pwsdd__ResolveMatches = NULL;
   struct wsdd__ResolveMatchesType *pwsdd__ResolveMatchesType = NULL;
   struct soap *pSoap = NULL;
   char *pAction=NULL, *pMessageID=NULL, *pTo=NULL;
   char *pEndpointAddress=NULL, *pTypes=NULL, *pItem=NULL, *pXAddrs=NULL, *pMatchBy=NULL;
            
   if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
      return 0;
      
#if WSDISCOVERY_SPEC_VER == WSDISCOVERY_SPEC_200901      
   pAction  = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ResolveMatches");
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
#else   
   pAction  = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/ResolveMatches");
   pMatchBy = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986");
#endif   
   pMessageID = nativeGetMessageId();
   pTo =nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   pXAddrs = nativeGetXAddrs(inet_ntoa(pSockAddr_In->sin_addr));
   
   
   pSoap = soap_new1(SOAP_IO_UDP);   
   pSoap->fsend = mysend;

   // Build SOAP Header
   soap_header(pSoap);
   pSoap->header->wsa5__Action = MySoapCopyString(pSoap, pAction);
   pSoap->header->wsa5__MessageID = MySoapCopyString(pSoap, pMessageID);
   pSoap->header->wsa5__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";
   pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
   soap_default_wsdd__AppSequenceType(pSoap, pSoap->header->wsdd__AppSequence);
   pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
   pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
   pSoap->header->wsdd__AppSequence->SequenceId = NULL;
   
   pSoap->header->wsa5__RelatesTo = (struct wsa5__RelatesToType *) soap_malloc(pSoap,sizeof(struct wsa5__RelatesToType));
   soap_default_wsa5__RelatesToType(pSoap, pSoap->header->wsa5__RelatesTo);
   pSoap->header->wsa5__RelatesTo->__item = MySoapCopyString(pSoap, pSenderMsgId);
   
   
   // Build ResolveMatches Message
   pwsdd__ResolveMatches = (struct __wsdd__ResolveMatches *) soap_malloc(pSoap,sizeof(struct __wsdd__ResolveMatches));
   soap_default___wsdd__ResolveMatches(pSoap, pwsdd__ResolveMatches);
   pwsdd__ResolveMatchesType = (struct wsdd__ResolveMatchesType *) soap_malloc(pSoap,sizeof(struct wsdd__ResolveMatchesType));
   soap_default_wsdd__ResolveMatchesType(pSoap, pwsdd__ResolveMatchesType);

   pwsdd__ResolveMatches->wsdd__ResolveMatches = pwsdd__ResolveMatchesType;   
   
   pwsdd__ResolveMatchesType->ResolveMatch = (struct wsdd__ResolveMatchType *)soap_malloc(pSoap, sizeof(struct wsdd__ResolveMatchType));   
   soap_default_wsdd__ResolveMatchType(pSoap, pwsdd__ResolveMatchesType->ResolveMatch);
   
   pwsdd__ResolveMatchesType->ResolveMatch->wsa5__EndpointReference.Address = MySoapCopyString(pSoap, pEndpointAddress);
   pwsdd__ResolveMatchesType->ResolveMatch->MetadataVersion = nativeGetMetadataVersion();
               
   soap_serializeheader(pSoap);

   soap_response(pSoap, SOAP_OK);
   soap_envelope_begin_out(pSoap);
   soap_putheader(pSoap);
   soap_body_begin_out(pSoap);
   vErr = soap_put___wsdd__ResolveMatches(pSoap, pwsdd__ResolveMatches, "-wsdd:ResolveMatches", "wsdd:ResolveMatchType");
   soap_body_end_out(pSoap);
   soap_envelope_end_out(pSoap);
   soap_end_send(pSoap);
   soap_destroy(pSoap);
   soap_end(pSoap);
   soap_free(pSoap);


   free(pAction);
   free(pMessageID);
   free(pTo);
   free(pEndpointAddress);
   free(pTypes);
   free(pItem);
   free(pXAddrs);
   free(pMatchBy);
   
   // Chapter 3.1.3
   // Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
   // This timer MUST be set to a random value between 0 and APP_MAX_DELAY   
   usleep( (random()%APP_MAX_DELAY) ); 
           
   char *pBuffer = getXmlBufferData();
   int vBufLen = strlen(pBuffer);      
   DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
   
   if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)pSockAddr_In, sizeof(*pSockAddr_In)) < 0)
   {
      perror("Sending datagram message error");
   }
   else
     DBG("Sending SendResolveMatches message...OK\n");   
     
   clearXmlBuffer();
     
   return SOAP_OK;
}

int SendFault(int socket, struct sockaddr_in *pSockAddr_In, char *pSenderMsgId)
{
   int vErr = 0;
   struct SOAP_ENV__Fault *pFault = NULL;
   struct soap *pSoap=NULL;
   char *pAction=NULL, *pMessageID=NULL, *pTo=NULL;
   char *pEndpointAddress=NULL, *pTypes=NULL, *pItem=NULL, *pXAddrs=NULL, *pMatchBy=NULL;
                           
   SOAP_NMAC struct Namespace namespaces_fault[] =
   {
      {"SOAP-ENV", "http://www.w3.org/2003/05/soap-envelope", "http://schemas.xmlsoap.org/soap/envelope/", NULL},
      {"SOAP-ENC", "http://www.w3.org/2003/05/soap-encoding", "http://schemas.xmlsoap.org/soap/encoding/", NULL},
      {"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
      {"wsa5", "http://schemas.xmlsoap.org/ws/2004/08/addressing", "http://www.w3.org/2005/08/addressing", NULL},
      {"wsdd", "http://schemas.xmlsoap.org/ws/2005/04/discovery", "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01", NULL},
      {NULL, NULL, NULL, NULL}
   };
            
   if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
      return 0;
      
#if WSDISCOVERY_SPEC_VER == WSDISCOVERY_SPEC_200901
   pAction  = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/fault");
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
#else      
   pAction  = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/fault");
   pMatchBy = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986");
#endif   
   pMessageID = nativeGetMessageId();
   pTo = nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   pXAddrs = nativeGetXAddrs(inet_ntoa(pSockAddr_In->sin_addr));
   
   pSoap = soap_new1(SOAP_IO_UDP);   
   pSoap->version = 2;
   pSoap->fsend = mysend;
   //pSoap->namespaces = namespaces_fault;

   // Build SOAP Header
   soap_header(pSoap);
   pSoap->header->wsa5__Action = MySoapCopyString(pSoap, pAction);
   pSoap->header->wsa5__MessageID = MySoapCopyString(pSoap, pMessageID);
   pSoap->header->wsa5__To = MySoapCopyString(pSoap, "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous");
   pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
   soap_default_wsdd__AppSequenceType(pSoap, pSoap->header->wsdd__AppSequence);
   pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
   pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
   pSoap->header->wsdd__AppSequence->SequenceId = NULL;
   if(pSenderMsgId!=NULL)
   {
      pSoap->header->wsa5__RelatesTo = (struct wsa5__RelatesToType *) soap_malloc(pSoap,sizeof(struct wsa5__RelatesToType));
      soap_default_wsa5__RelatesToType(pSoap, pSoap->header->wsa5__RelatesTo);
      pSoap->header->wsa5__RelatesTo->__item = MySoapCopyString(pSoap, pSenderMsgId);
   }
   
   
   // Build Fault Message
   
   // malloc pSoap->fault, pSoap->fault->SOAP_ENV__Code, pSoap->fault->SOAP_ENV__Reason
   soap_fault(pSoap);
   pSoap->fault->SOAP_ENV__Code->SOAP_ENV__Value = MySoapCopyString(pSoap, "SOAP-ENV:Sender");
   
   // malloc pSoap->fault, pSoap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode
   soap_faultsubcode(pSoap);
   pSoap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value = MySoapCopyString(pSoap, "wsdd:MatchingRuleNotSupported");
   pSoap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode = NULL;
   pSoap->fault->SOAP_ENV__Reason->SOAP_ENV__Text = MySoapCopyString(pSoap, "the matching rule specified is not supported");
   
   // malloc pSoap->fault, pSoap->fault->SOAP_ENV__Detail
   soap_faultdetail(pSoap);
   {
      int vLen = 0;
      char *pTmp = NULL;
      vLen = strlen(pItem);
      pTmp = malloc(vLen + 100);
      memset(pTmp, 0, vLen+100);
      sprintf(pTmp, "<wsdd:SupportedMatchingRules>%s</wsdd:SupportedMatchingRules>", pItem);   
      pSoap->fault->SOAP_ENV__Detail->__any = MySoapCopyString(pSoap, pTmp);
      free(pTmp);
   }   
   
   // mimic soap_send_fault()
   soap_serializeheader(pSoap);
   soap_serializefault(pSoap);
   soap_begin_count(pSoap);
   
   soap_response(pSoap, SOAP_FAULT);
   soap_envelope_begin_out(pSoap);
   soap_putheader(pSoap);
   soap_body_begin_out(pSoap);
   soap_putfault(pSoap);
   soap_body_end_out(pSoap);
   soap_envelope_end_out(pSoap);
   soap_end_send(pSoap);

   soap_destroy(pSoap);
   soap_end(pSoap);
   soap_free(pSoap);

   free(pAction);
   free(pMessageID);
   free(pTo);
   free(pEndpointAddress);
   free(pTypes);
   free(pItem);
   free(pXAddrs);
   free(pMatchBy);
      
   // Chapter 3.1.3
   // Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
   // This timer MUST be set to a random value between 0 and APP_MAX_DELAY         
   usleep( (random()%APP_MAX_DELAY) ); 
   
   char *pBuffer = getXmlBufferData();
   int vBufLen = strlen(pBuffer);      
   //DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);

   if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)pSockAddr_In, sizeof(*pSockAddr_In)) < 0)
   {
      perror("Sending datagram message error");
   }
   else
     DBG("Sending SendFault message...OK\n");   
  
   clearXmlBuffer();     
   return SOAP_OK;
}

// Receive Multicast request and send unicast response (Probe and Resolve)
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Probe(struct soap *pSoap, struct wsdd__ProbeType *wsdd__Probe)
{
   int vSocket=-1;
   int bScopeValid=1, bMatchValid=0, bMulticast=1;
   
   if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
   {
      DBG("%s %s :%d NONDISCOVERABLE\n",__FILE__,__func__, __LINE__);
      return SOAP_OK;
   }

   // The ipaddress may changed
   initMyIpString();
   bMulticast = GetMulticastFlag();
   
   if(wsdd__Probe->Scopes)
   {
      if(wsdd__Probe->Scopes->__item)
      {
         if(wsdd__Probe->Scopes->MatchBy)
         {
            if(strcmp(wsdd__Probe->Scopes->MatchBy,"http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986")==0)
            {
               bScopeValid = match_rfc3986(wsdd__Probe->Scopes->__item);
            }
            else if(strcmp(wsdd__Probe->Scopes->MatchBy,"http://schemas.xmlsoap.org/ws/2005/04/discovery/uuid")==0)
            {
               bScopeValid = match_uuid(wsdd__Probe->Scopes->__item);
            }
            else if(strcmp(wsdd__Probe->Scopes->MatchBy,"http://schemas.xmlsoap.org/ws/2005/04/discovery/ldap")==0)
            {
               bScopeValid = match_ldap(wsdd__Probe->Scopes->__item);
            }
            else if(strcmp(wsdd__Probe->Scopes->MatchBy,"http://schemas.xmlsoap.org/ws/2005/04/discovery/strcmp0")==0)
            {
               bScopeValid = match_strcmp0(wsdd__Probe->Scopes->__item);
            }                  
            else if(strcmp(wsdd__Probe->Scopes->MatchBy,"http://schemas.xmlsoap.org/ws/2005/04/discovery/none")==0)
            {
               bScopeValid = match_none(wsdd__Probe->Scopes->__item);
            }            
            else
            {
               bMatchValid=0;
            }     
         }
         else
         {
            if(bMulticast)
            {
               // For Multicast probe
               // If there is no MatchBy in Scope Tag
               // The default match is http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986                  
               bScopeValid = match_rfc3986(wsdd__Probe->Scopes->__item);   
               bMatchValid = 1;
            }
            else
            {
               bMatchValid = 0;
            }
         }
      }
   }
   
   vSocket = CreateUnicastClient(&pSoap->peer,MULTICAST_PORT);  
   if(bMulticast)
   {
      if(bScopeValid)
      {
         char *pSenderMessageId=NULL;
         if(pSoap->header)
         {
            if(pSoap->header->wsa5__MessageID)
            {
               pSenderMessageId = pSoap->header->wsa5__MessageID;
            }
         }
         SendProbeMatches(vSocket, &pSoap->peer, pSenderMessageId);
         SendProbeMatches(vSocket, &pSoap->peer, pSenderMessageId);
      }
      else
      {
         DBG( "%s %s :%d\n",__FILE__,__func__, __LINE__);
         // Chapter 6.3.1 Target Service
         // If a Target Service receives a Probe that does not match, it MUST NOT respond with a Probe Match.

      }
   }
   else
   {
      if(bMatchValid == 0)
      {
         // Chapter 7.3.6 SOAP Fault Messages
         // If a device receives a unicast Probe message and it does not support the matching rule, then
         // the device may choose not to send a Probe Match, and instead generate a SOAP fault bound
         // to SOAP 1.2 as follows:
         /*
            [action] http://schemas.xmlsoap.org/ws/2005/04/discovery/fault
            [Code] s12:Sender
            [Subcode] d:MatchingRuleNotSupported
            [Reason] E.g., the matching rule specified is not supported
            [Detail] 
            <d: SupportedMatchingRules>
            List of xs:anyURI
            </d: SupportedMatchingRules>
         */           
         char *pSenderMessageId=NULL;
         if(pSoap->header)
         {
            if(pSoap->header->wsa5__MessageID)
            {
               pSenderMessageId = pSoap->header->wsa5__MessageID;
            }
         }                          
         SendFault(vSocket, &pSoap->peer, pSenderMessageId);
      }
      else if(bScopeValid)
      {
         char *pSenderMessageId=NULL;
         if(pSoap->header)
         {
            if(pSoap->header->wsa5__MessageID)
            {
               pSenderMessageId = pSoap->header->wsa5__MessageID;
            }
         }
         SendProbeMatches(vSocket, &pSoap->peer, pSenderMessageId);
         SendProbeMatches(vSocket, &pSoap->peer, pSenderMessageId);
      }
   }

   close(vSocket);
   return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap *pSoap, struct wsdd__ResolveType *wsdd__Resolve)
{
   int vSocket=-1;   
   int bResolveValid=1;
      
   if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
   {
      DBG("%s %s :%d NONDISCOVERABLE\n",__FILE__,__func__, __LINE__);
      return SOAP_OK;
   }
      
   // Check peer for ad hoc mode or managed mode
   if(wsdd__Resolve)
   {
      if(wsdd__Resolve->wsa5__EndpointReference.Address)
      {
         // TODO: Check if resolve match;
         
      }
   }
   
   vSocket = CreateUnicastClient(&pSoap->peer, MULTICAST_PORT);
   if(bResolveValid)
   {
      char *pSenderMessageId=NULL;
      if(pSoap->header)
         if(pSoap->header->wsa5__MessageID)
            pSenderMessageId = pSoap->header->wsa5__MessageID;
            
      SendResolveMatches(vSocket, &pSoap->peer, pSenderMessageId);
   }
   else
   {
      // Chapter 6.3.1 Target Service
      // If a Target Service receives a Resolve that does not match, it MUST NOT respond with a Resolve Match.
      ;
   }
   close(vSocket);   
   return SOAP_OK;
}


// Unused callback function
SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *pSoap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
   DBG("%s %s :%d\n",__FILE__,__func__, __LINE__);
   return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap *pSoap, struct wsdd__HelloType *wsdd__Hello)
{
   DBG("%s %s :%d\n",__FILE__,__func__, __LINE__);
   return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap *pSoap, struct wsdd__ByeType *wsdd__Bye)
{
   DBG("%s %s :%d\n",__FILE__,__func__, __LINE__);
   return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap *pSoap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{
   DBG("%s %s :%d sendfd=%d\n",__FILE__,__func__, __LINE__, pSoap->sendfd);
   return SOAP_OK;
}   

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap *pSoap, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{
   DBG("%s %s :%d\n",__FILE__,__func__, __LINE__);
   return SOAP_OK;
}
