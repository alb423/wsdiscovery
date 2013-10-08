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
int SendHello(int socket)
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
	pAction = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Hello");
	pMessageID = nativeGetMessageId();
	pTo =nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   pXAddrs = nativeGetXAddrs(inet_ntoa(gMSockAddr.sin_addr));
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
   	
	pSoap = soap_new1(SOAP_IO_UDP);		
	pSoap->fsend = mysend;
	
	// Build SOAP Header
	soap_header(pSoap);
	pSoap->header->wsa5__Action = MySoapCopyString(pSoap, pAction);
	pSoap->header->wsa5__MessageID = MySoapCopyString(pSoap, pMessageID);
	pSoap->header->wsa5__To = MySoapCopyString(pSoap, pTo);
	
	// This part cause segmentation fault, because align at 4-, 8- or 16-byte boundary
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

	
	char *pBuffer = getXmlBufferData();
	int vBufLen = strlen(pBuffer);
	DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
	
	if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	  DBG("Sending datagram message...OK\n");		
	  
	clearXmlBuffer();
	  
	return SOAP_OK;
}

int SendBye(int socket)
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
	pAction = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Bye");
	pMessageID = nativeGetMessageId();
	pTo =nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   pXAddrs = nativeGetXAddrs(inet_ntoa(gMSockAddr.sin_addr));
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
   		
	pSoap = soap_new1(SOAP_IO_UDP);		
	pSoap->fsend = mysend;
	
	// Build SOAP Header
	soap_header(pSoap);
	pSoap->header->wsa5__Action = MySoapCopyString(pSoap, pAction);
	pSoap->header->wsa5__MessageID = MySoapCopyString(pSoap, pMessageID);
	pSoap->header->wsa5__To = MySoapCopyString(pSoap, pTo);
	
	// This part cause segmentation fault, because align at 4-, 8- or 16-byte boundary
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

   
	char *pBuffer = getXmlBufferData();
	int vBufLen = strlen(pBuffer);	
	DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
	
	if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	  DBG("Sending datagram message...OK\n");	

	clearXmlBuffer();	  	
		  
	return SOAP_OK;
}


// 20130919
int SendProbe(int socket)
{
	int vErr = 0;
	struct __wsdd__Probe *pWsdd__Probe = MyMalloc(sizeof(struct __wsdd__Probe));
	struct wsdd__ProbeType *pWsdd__ProbeType = MyMalloc(sizeof(struct wsdd__ProbeType));	
	struct soap *pSoap=NULL;
	
	pSoap = soap_new1(SOAP_IO_UDP);
	pSoap->fsend = mysend;

	// Build SOAP Header
	soap_header(pSoap);
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Probe");
	pSoap->header->wsa5__MessageID = nativeGetMessageId();
	pSoap->header->wsa5__To = nativeGetTo();
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
	pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
	pSoap->header->wsdd__AppSequence->SequenceId = NULL;

	// Build Hello Message
	soap_default___wsdd__Probe(pSoap, pWsdd__Probe);
	pSoap->encodingStyle = NULL;
	
	pWsdd__Probe->wsdd__Probe = pWsdd__ProbeType;   
	pWsdd__ProbeType->Types = nativeGetTypes();
	pWsdd__ProbeType->Scopes = (struct wsdd__ScopesType *)soap_malloc(pSoap, sizeof(struct wsdd__ScopesType));
	pWsdd__ProbeType->Scopes->MatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
	#if 1
		pWsdd__ProbeType->Scopes->__item = nativeGetScopesItem();
	#else
		// Test for invalid scope
		pWsdd__ProbeType->Scopes->__item = CopyString("undefinedScope");
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
   		
	char *pBuffer = getXmlBufferData();
	int vBufLen = strlen(pBuffer);		
	DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
	
	if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	  DBG("Sending datagram message...OK\n");	
	  
	clearXmlBuffer();	  
	return SOAP_OK;
}

int SendResolve(int socket)
{
	int vErr = 0;
	struct __wsdd__Resolve *pWsdd__Resolve = MyMalloc(sizeof(struct __wsdd__Resolve));
	struct wsdd__ResolveType *pWsdd__ResolveType = MyMalloc(sizeof(struct wsdd__ResolveType));	
	struct soap *pSoap=NULL;

	pSoap = soap_new1(SOAP_IO_UDP);
	pSoap->fsend = mysend;

	// Build SOAP Header
	soap_header(pSoap);
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Resolve");
	pSoap->header->wsa5__MessageID = nativeGetMessageId();
	pSoap->header->wsa5__To = nativeGetTo();
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
	pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
	pSoap->header->wsdd__AppSequence->SequenceId = NULL;
	
	// Build Resolve Message
	soap_default___wsdd__Resolve(pSoap, pWsdd__Resolve);
	pSoap->encodingStyle = NULL;
	
	pWsdd__Resolve->wsdd__Resolve = pWsdd__ResolveType;   
   pWsdd__ResolveType->wsa5__EndpointReference.Address = nativeGetEndpointAddress();
	
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
	
	char *pBuffer = getXmlBufferData();
	int vBufLen = strlen(pBuffer);	
	DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
	
	if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)&gMSockAddr, sizeof(gMSockAddr)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	  DBG("Sending datagram message...OK\n");	
	  
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
		
	pAction = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches");
	pMessageID = nativeGetMessageId();
	pTo = nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   pXAddrs = nativeGetXAddrs(inet_ntoa(pSockAddr_In->sin_addr));
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
	
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
		
	char *pBuffer = getXmlBufferData();
	int vBufLen = strlen(pBuffer);		
	DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
	
	if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)pSockAddr_In, sizeof(struct sockaddr_in)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	  DBG("Sending datagram message...OK\n");	
	  
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
		
	pAction = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ResolveMatches");
	pMessageID = nativeGetMessageId();
	pTo =nativeGetTo();
   pEndpointAddress = nativeGetEndpointAddress();
   pTypes = nativeGetTypes();
   pItem = nativeGetScopesItem();
   pXAddrs = nativeGetXAddrs(inet_ntoa(pSockAddr_In->sin_addr));
   pMatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");
	
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
	
	   		
	char *pBuffer = getXmlBufferData();
	int vBufLen = strlen(pBuffer);		
	DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
	
	if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)pSockAddr_In, sizeof(*pSockAddr_In)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	  DBG("Sending datagram message...OK\n");	
	  
	clearXmlBuffer();
	  
	return SOAP_OK;
}

int SendFault(int socket, struct sockaddr_in *pSockAddr_In)
{
	int vErr = 0;
	struct SOAP_ENV__Fault *pFault = MyMalloc(sizeof(struct SOAP_ENV__Fault));
	struct soap *pSoap=NULL;

	pSoap = soap_new1(SOAP_IO_UDP);
	pSoap->fsend = mysend;

	// Build SOAP Header
	soap_header(pSoap);
	pSoap->header->wsa5__Action = CopyString("http://schemas.xmlsoap.org/ws/2005/04/discovery/fault");
	pSoap->header->wsa5__MessageID = nativeGetMessageId();
	pSoap->header->wsa5__To = nativeGetTo();
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) soap_malloc(pSoap,sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
	pSoap->header->wsdd__AppSequence->MessageNumber = nativeGetMessageNumber();
	pSoap->header->wsdd__AppSequence->SequenceId = NULL;

	// Build Fault Message
	pFault->SOAP_ENV__Code = (struct SOAP_ENV__Code*)MyMalloc(sizeof(struct SOAP_ENV__Code));
	pFault->SOAP_ENV__Code->SOAP_ENV__Value = CopyString("SOAP-ENV:Sender");
	pFault->SOAP_ENV__Code->SOAP_ENV__Subcode = (struct SOAP_ENV__Code*)MyMalloc(sizeof(struct SOAP_ENV__Code));
	pFault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value = CopyString("d:MatchingRuleNotSupported");
	pFault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode = NULL;
	
	pFault->SOAP_ENV__Reason = (struct SOAP_ENV__Reason*)MyMalloc(sizeof(struct SOAP_ENV__Reason));
	pFault->SOAP_ENV__Reason->SOAP_ENV__Text = CopyString("the matching rule specified is not supported");
	
	soap_serializeheader(pSoap);

	soap_response(pSoap, SOAP_FAULT);
	soap_envelope_begin_out(pSoap);
	soap_putheader(pSoap);
	soap_body_begin_out(pSoap);
	
	vErr = soap_put_SOAP_ENV__Fault(pSoap, pFault, "SOAP-ENV:Fault", "SOAP-ENV:Fault");
	
	soap_body_end_out(pSoap);
	soap_envelope_end_out(pSoap);
	soap_end_send(pSoap);
	soap_destroy(pSoap);
	soap_end(pSoap);
   soap_free(pSoap);
   		
	char *pBuffer = getXmlBufferData();
	int vBufLen = strlen(pBuffer);		
	DBG("vErr=%d, Len=%d, Buf=\n%s\n", vErr, vBufLen, pBuffer);
	
	if(sendto(socket, pBuffer, vBufLen, 0, (struct sockaddr*)pSockAddr_In, sizeof(*pSockAddr_In)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	  DBG("Sending datagram message...OK\n");	
	  
	clearXmlBuffer();	  
	return SOAP_OK;
}

// Receive Multicast request and send unicast response (Probe and Resolve)
SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Probe(struct soap *pSoap, struct wsdd__ProbeType *wsdd__Probe)
{
	int vSocket=-1;
	int bScopeValid=1;
	fprintf(stderr, "%s %s :%d sendfd=%d\n",__FILE__,__func__, __LINE__, pSoap->sendfd);
	if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
	{
		fprintf(stderr, "%s %s :%d NONDISCOVERABLE\n",__FILE__,__func__, __LINE__);
		return SOAP_OK;
	}
			
#if 0	
	fprintf(stderr, "\n======\n");
	fprintf(stderr, "vLen=%zd, buf=\n%s\n", pSoap->buflen,pSoap->buf);
	if(wsdd__Probe->Types)
		fprintf(stderr, "Types=%s\n",wsdd__Probe->Types);	
	if(wsdd__Probe->Scopes)
	{
		fprintf(stderr, "Scopes->__item=%s\n",wsdd__Probe->Scopes->__item);	
		fprintf(stderr, "Scopes->MatchBy=%s\n",wsdd__Probe->Scopes->MatchBy);	
	}
	fprintf(stderr, "======\n\n");
#endif	

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
			}
			else
			{
				// If there is no MatchBy in Scope Tag
				// The default match is http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc3986
				bScopeValid = match_rfc3986(wsdd__Probe->Scopes->__item);	
			}
		}
	}
	
	// For ONVIF only
	if(wsdd__Probe->Types)
	{
		char *pType = nativeGetTypes();
		if(strcmp(wsdd__Probe->Types, pType)==0)
		{
			bScopeValid = 1;
		}
		else
		{
			bScopeValid = 0;
		}			
		free(pType);
      printf("wsdd__Probe->Types = %s\n",wsdd__Probe->Types);
	}
	
	// Test only
	//bScopeValid = 1;
	
	
	// Chapter 3.1.3
	// Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
	// This timer MUST be set to a random value between 0 and APP_MAX_DELAY
	usleep( (random()%APP_MAX_DELAY) );
	// sleep(1);
	
	// For IPv4 only
	vSocket = CreateUnicastClient(&pSoap->peer);
	if(bScopeValid)
	{
		char *pSenderMessageId=NULL;
		if(pSoap->header)
		{
			if(pSoap->header->wsa5__MessageID)
			{
			   pSenderMessageId = pSoap->header->wsa5__MessageID;
			   #if 0
				int vLen=0;
				vLen = strlen(pSoap->header->wsa5__MessageID);
				pSenderMessageId = MyMalloc(vLen+10);
				sprintf(pSenderMessageId, "%s",pSoap->header->wsa5__MessageID);
				#endif
			}
      }
      fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
		SendProbeMatches(vSocket, &pSoap->peer, pSenderMessageId);
		usleep(500000);
		fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
		SendProbeMatches(vSocket, &pSoap->peer, pSenderMessageId);
		
		//if(pSenderMessageId)
		//	free(pSenderMessageId);
	}
	else
	{
	   fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
		// Chapter 6.3.1 Target Service
		// If a Target Service receives a Probe that does not match, it MUST NOT respond with a Probe Match.
		// SendFault(vSocket, &pSoap->peer);
	}
	close(vSocket);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap *pSoap, struct wsdd__ResolveType *wsdd__Resolve)
{
	int vSocket=-1;	
	int bResolveValid=1;
		
	fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
	if(nativeGetDiscoveryMode() == NONDISCOVERABLE )
	{
		fprintf(stderr, "%s %s :%d NONDISCOVERABLE\n",__FILE__,__func__, __LINE__);
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
	
	
	vSocket = CreateUnicastClient(&pSoap->peer);
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
	fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap *pSoap, struct wsdd__HelloType *wsdd__Hello)
{
	fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap *pSoap, struct wsdd__ByeType *wsdd__Bye)
{
	fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap *pSoap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{
	fprintf(stderr, "%s %s :%d sendfd=%d\n",__FILE__,__func__, __LINE__, pSoap->sendfd);
	return SOAP_OK;
}	

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap *pSoap, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{
	fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);
	return SOAP_OK;
}
