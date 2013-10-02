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

// Send Multicast Packet (Hello and Bye)
int SendHello(int socket)
{
	int vErr = 0;
	struct __wsdd__Hello *pWsdd__Hello = MyMalloc(sizeof(struct __wsdd__Hello));
	struct wsdd__HelloType *pWsdd__HelloType = MyMalloc(sizeof(struct wsdd__HelloType));	
	struct soap *pSoap=MyMalloc(sizeof(struct soap));
	soap_init1(pSoap,SOAP_IO_UDP);
	
	pSoap->fsend = mysend;

	// Build SOAP Header
	pSoap->header = (struct SOAP_ENV__Header *) MyMalloc(sizeof(struct SOAP_ENV__Header));
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Hello");
	pSoap->header->wsa5__MessageID = CopyString("urn:uuid:73948edc-3204-4455-bae2-7c7d0ff6c37c");
	pSoap->header->wsa5__To = CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) MyMalloc(sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
	pSoap->header->wsdd__AppSequence->MessageNumber = 1;

	// Build Hello Message
	soap_default___wsdd__Hello(pSoap, pWsdd__Hello);
	pSoap->encodingStyle = NULL;
	
	pWsdd__Hello->wsdd__Hello = pWsdd__HelloType;   
	pWsdd__HelloType->wsa5__EndpointReference.Address = nativeGetUUID();
	pWsdd__HelloType->Types = nativeGetTypes();
	
	// TODO: we can have many scopes 
	pWsdd__HelloType->Scopes = MyMalloc(sizeof(struct wsdd__ScopesType));
	pWsdd__HelloType->Scopes->MatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");; 
	pWsdd__HelloType->Scopes->__item = CopyString("onvif://www.onvif.org/Profile/Streaming");
	pWsdd__HelloType->XAddrs = nativeGetUUID();
	pWsdd__HelloType->MetadataVersion = 0;
		
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
	struct __wsdd__Bye *pWsdd__Bye = MyMalloc(sizeof(struct __wsdd__Bye));
	struct wsdd__ByeType *pWsdd__ByeType = MyMalloc(sizeof(struct wsdd__ByeType));	
	struct soap *pSoap=MyMalloc(sizeof(struct soap));
	soap_init1(pSoap,SOAP_IO_UDP);

	pSoap->fsend = mysend;

	// Build SOAP Header
	pSoap->header = (struct SOAP_ENV__Header *) MyMalloc(sizeof(struct SOAP_ENV__Header));
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Bye");
	pSoap->header->wsa5__MessageID = CopyString("urn:uuid:73948edc-3204-4455-bae2-7c7d0ff6c37c");
	pSoap->header->wsa5__To = CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) MyMalloc(sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
	pSoap->header->wsdd__AppSequence->MessageNumber = 1;

	// Build Bye Message
	soap_default___wsdd__Bye(pSoap, pWsdd__Bye);
	pSoap->encodingStyle = NULL;
	
	pWsdd__Bye->wsdd__Bye = pWsdd__ByeType;   
	pWsdd__ByeType->wsa5__EndpointReference.Address = nativeGetUUID();
	pWsdd__ByeType->Types = nativeGetTypes();
	pWsdd__ByeType->Scopes = MyMalloc(sizeof(struct wsdd__ScopesType));
	pWsdd__ByeType->Scopes->MatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");; 
	pWsdd__ByeType->Scopes->__item = CopyString("onvif://www.onvif.org/Profile/Streaming");
	pWsdd__ByeType->XAddrs = nativeGetUUID();
	pWsdd__ByeType->MetadataVersion = 0;
				   
	
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
	struct soap *pSoap=MyMalloc(sizeof(struct soap));
	soap_init1(pSoap,SOAP_IO_UDP);

	pSoap->fsend = mysend;

	// Build SOAP Header
	pSoap->header = (struct SOAP_ENV__Header *) MyMalloc(sizeof(struct SOAP_ENV__Header));
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Probe");
	pSoap->header->wsa5__MessageID = CopyString("urn:uuid:73948edc-3204-4455-bae2-7c7d0ff6c37c");
	pSoap->header->wsa5__To = CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) MyMalloc(sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
	pSoap->header->wsdd__AppSequence->MessageNumber = 1;

	// Build Hello Message
	soap_default___wsdd__Probe(pSoap, pWsdd__Probe);
	pSoap->encodingStyle = NULL;
	
	pWsdd__Probe->wsdd__Probe = pWsdd__ProbeType;   
	pWsdd__ProbeType->Types = nativeGetTypes();
	pWsdd__ProbeType->Scopes = MyMalloc(sizeof(struct wsdd__ScopesType));
	pWsdd__ProbeType->Scopes->MatchBy = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/rfc3986");; 
	pWsdd__ProbeType->Scopes->__item = CopyString("onvif://www.onvif.org/Profile/Streaming");
				   
	
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
	struct soap *pSoap=MyMalloc(sizeof(struct soap));
	soap_init1(pSoap,SOAP_IO_UDP);

	pSoap->fsend = mysend;

	// Build SOAP Header
	pSoap->header = (struct SOAP_ENV__Header *) MyMalloc(sizeof(struct SOAP_ENV__Header));
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Resolve");
	pSoap->header->wsa5__MessageID = CopyString("urn:uuid:73948edc-3204-4455-bae2-7c7d0ff6c37c");
	pSoap->header->wsa5__To = CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) MyMalloc(sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
	pSoap->header->wsdd__AppSequence->MessageNumber = 1;

	// Build Resolve Message
	soap_default___wsdd__Resolve(pSoap, pWsdd__Resolve);
	pSoap->encodingStyle = NULL;
	
	pWsdd__Resolve->wsdd__Resolve = pWsdd__ResolveType;   
  pWsdd__ResolveType->wsa5__EndpointReference.Address = nativeGetUUID();
	
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


int SendProbeMatches(int socket, struct sockaddr_in *pSockAddr_In)
{	
	int vErr = 0;
	struct __wsdd__ProbeMatches *pwsdd__ProbeMatches = MyMalloc(sizeof(struct __wsdd__ProbeMatches));
	struct wsdd__ProbeMatchesType *pwsdd__ProbeMatchesType = MyMalloc(sizeof(struct wsdd__ProbeMatchesType));	
	struct soap *pSoap=MyMalloc(sizeof(struct soap));
	soap_init1(pSoap,SOAP_IO_UDP);

	pSoap->fsend = mysend;


	// Build SOAP Header
	pSoap->header = (struct SOAP_ENV__Header *) MyMalloc(sizeof(struct SOAP_ENV__Header));
	memset(pSoap->header, 0, sizeof(struct SOAP_ENV__Header));
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches");
	pSoap->header->wsa5__MessageID = CopyString("urn:uuid:73948edc-3204-4455-bae2-7c7d0ff6c37c");
	pSoap->header->wsa5__To = CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) MyMalloc(sizeof(struct wsdd__AppSequenceType));
	memset(pSoap->header->wsdd__AppSequence, 0, sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
	pSoap->header->wsdd__AppSequence->MessageNumber = 1;

	pSoap->header->wsa5__ReplyTo = MyMalloc(sizeof(struct wsa5__EndpointReferenceType));
	memset(pSoap->header->wsa5__ReplyTo, 0, sizeof(struct wsa5__EndpointReferenceType));
	pSoap->header->wsa5__ReplyTo->Address = nativeGetUUID();
	
	// Build ProbeMatch Message
	soap_default___wsdd__ProbeMatches(pSoap, pwsdd__ProbeMatches);
	pSoap->encodingStyle = NULL;
	
	// TODO: make a real match procedure
	pwsdd__ProbeMatches->wsdd__ProbeMatches = pwsdd__ProbeMatchesType;   
	pwsdd__ProbeMatchesType->__sizeProbeMatch = 1;
	pwsdd__ProbeMatchesType->ProbeMatch = MyMalloc(sizeof(struct wsdd__ProbeMatchType));
	pwsdd__ProbeMatchesType->ProbeMatch->wsa5__EndpointReference.Address = nativeGetUUID();
	pwsdd__ProbeMatchesType->ProbeMatch->MetadataVersion = 1234;
				   
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

int SendResolveMatches(int socket, struct sockaddr_in *pSockAddr_In)
{
	int vErr = 0;
	struct __wsdd__ResolveMatches *pwsdd__ResolveMatches = MyMalloc(sizeof(struct __wsdd__ResolveMatches));
	struct wsdd__ResolveMatchesType *pwsdd__ResolveMatchesType = MyMalloc(sizeof(struct wsdd__ResolveMatchesType));	
	struct soap *pSoap=MyMalloc(sizeof(struct soap));
	soap_init1(pSoap,SOAP_IO_UDP);

	pSoap->fsend = mysend;

	// Build SOAP Header
	pSoap->header = (struct SOAP_ENV__Header *) MyMalloc(sizeof(struct SOAP_ENV__Header));
	pSoap->header->wsa5__Action = CopyString("http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ResolveMatches");
	pSoap->header->wsa5__MessageID = CopyString("urn:uuid:73948edc-3204-4455-bae2-7c7d0ff6c37c");
	pSoap->header->wsa5__To = CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	pSoap->header->wsdd__AppSequence = (struct wsdd__AppSequenceType *) MyMalloc(sizeof(struct wsdd__AppSequenceType));
	pSoap->header->wsdd__AppSequence->InstanceId = nativeGetInstanceId();
	pSoap->header->wsdd__AppSequence->MessageNumber = 1;

	// Build Hello Message
	soap_default___wsdd__ResolveMatches(pSoap, pwsdd__ResolveMatches);
	pSoap->encodingStyle = NULL;
	
	pwsdd__ResolveMatches->wsdd__ResolveMatches = pwsdd__ResolveMatchesType;   
	pwsdd__ResolveMatchesType->ResolveMatch = MyMalloc(sizeof(struct wsdd__ResolveMatchType));	
	pwsdd__ResolveMatchesType->ResolveMatch->wsa5__EndpointReference.Address = nativeGetUUID();
	pwsdd__ResolveMatchesType->ResolveMatch->MetadataVersion = 1234;
				   
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
	fprintf(stderr, "%s %s :%d sendfd=%d\n",__FILE__,__func__, __LINE__, pSoap->sendfd);
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

#if 0
	//fprintf(stderr, "socket=%d, sendsk=%d, recvsk=%d, recvfd=%d, sendfd=%d\n",pSoap->socket,pSoap->sendsk,pSoap->recvsk,pSoap->recvfd,pSoap->sendfd);
	{		
		fprintf(stderr,"sin_addr.s_addr=%s, sin_port=%d\n", inet_ntoa(pSoap->peer.sin_addr), pSoap->peer.sin_port);	
		fprintf(stderr,"soap->ip=%ld\n",pSoap->ip);
	}
#endif	
	
	// For IPv4 only
	vSocket = CreateUnicastClient(&pSoap->peer);
	SendProbeMatches(vSocket, &pSoap->peer);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap *pSoap, struct wsdd__ResolveType *wsdd__Resolve)
{
	int vSocket=-1;	
	fprintf(stderr, "%s %s :%d\n",__FILE__,__func__, __LINE__);

	vSocket = CreateUnicastClient(&pSoap->peer);
	SendResolveMatches(vSocket, &pSoap->peer);
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
