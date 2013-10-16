#ifndef PORTING_H
#define PORTING_H

#define DISCOVERABLE 0
#define NONDISCOVERABLE 1


// The wsdiscovery has 2 versions, you should choose what spec you need.
// For example:
// 2009/01 spec
// probe Action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches"
// 2005/04 spec
// probe Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches"
#define WSDISCOVERY_SPEC_200504 1
#define WSDISCOVERY_SPEC_200901 2
#define WSDISCOVERY_SPEC_VER WSDISCOVERY_SPEC_200504
   
// Chapter 3.1.3
// Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
// This timer MUST be set to a random value between 0 and APP_MAX_DELAY
#define APP_MAX_DELAY 500

extern char *nativeGetEndpointAddress();
extern char *nativeGetXAddrs(char *pAddrToResponse);
extern char *nativeGetTypes();
extern char *nativeGetScopesItem();
extern char *nativeGetMessageId();
extern char *nativeGetTo();

extern int nativeGetMessageNumber();
extern int nativeGetInstanceId();

extern int nativeGetMetadataVersion();
extern void nativeIncreaseMetadataVersion();

extern int nativeGetDiscoveryMode();
extern void nativeChangeDiscoveryMode(char Mode);

#endif

