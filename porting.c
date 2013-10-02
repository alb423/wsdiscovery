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

char *nativeGetUUID()
{
	return CopyString("urn:uuid:98190dc2-0890-4ef8-ac9a-5940995e6119");
}

char *nativeGetTypes()
{
	return CopyString("tds:Device");
}


int nativeGetInstanceId()
{
	// TODO: build a number generator
	return 1077004800;
}