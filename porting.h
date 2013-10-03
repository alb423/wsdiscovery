#ifndef PORTING_H
#define PORTING_H

#define DISCOVERABLE 0
#define NONDISCOVERABLE 1

// Chapter 3.1.3
// Before sending somemessage types. Target ServiceMust wait for a timer to elapse before sending the message.
// This timer MUST be set to a random value between 0 and APP_MAX_DELAY
#define APP_MAX_DELAY 500



extern char *nativeGetEndpointAddress();
extern char *nativeGetXAddrs();
extern char *nativeGetTypes();
extern char *nativeGetScopesItem();
extern char *nativeGetMessageId();
extern char *nativeGetTo();

extern int nativeGetInstanceId();

extern int nativeGetMetadataVersion();
extern void nativeIncreaseMetadataVersion();

extern int nativeGetDiscoveryMode();
extern void nativeChangeDiscoveryMode(char Mode);

#endif

