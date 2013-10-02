#include <stdio.h>
#include <time.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <unistd.h>

#include "util.h"
;
char *nativeGetXAddrs()
{
	return CopyString("urn:uuid:98190dc2-0890-4ef8-ac9a-5940995e6119");
}

char *nativeGetEndpointAddress()
{
	return CopyString("urn:uuid:98190dc2-0890-4ef8-ac9a-5940995e6119");
}

char *nativeGetTypes()
{
	// For old version, return "dn:NetworkVideoTransmitter"
	return CopyString("tds:Device");
}

char *nativeGetScopes()
{
	return CopyString("\
onvif://www.onvif.org/type/audio_encoder \
onvif://www.onvif.org/type/video_encoder \
onvif://www.onvif.org/name/albert \
onvif://www.onvif.org/hardware/undefined \
onvif://www.onvif.org/location/Taiwan \
onvif://www.onvif.org/Profile/Streaming ");
}

int nativeGetInstanceId()
{
	// TODO: build a number generator
	return 1077004800;
}

int nativeGetMetadataVersion()
{
	return 1234;
}