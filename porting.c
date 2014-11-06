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
#include "porting.h"

static int _gMetadataVersion = 1;
static int _gDiscoveryMode = 0;
static int _gInstanceId = 1;
static int _gMessageNumber = 1;
char *_gpScopeData=NULL;

int nativeGetMessageNumber()
{
	return _gMessageNumber++;
}

char *nativeGetXAddrs(char *pAddrToResponse)
{
	int i = 0, j = 0, vLen = 0, vSuffix = 0;
	char pTmp[1024] = {0};
	char *pAddrToSend = NULL;
	
	// Resonse the ip with the same netmask
   for(i=0; i<NET_MAX_INTERFACE; i++)
   {
      pAddrToSend = gpLocalAddr[i];
      vLen = strlen(pAddrToSend);
      if(vLen>0)
      {
         for(j=vLen-1;j>=0;j--)
         {
            if(pAddrToSend[j]=='.')
               break;
            vSuffix++;
         }

         if(strncmp(pAddrToSend, pAddrToResponse, vLen-vSuffix)==0)
            break;
      }
   }
   
   if(strlen(pAddrToSend)==0)
      pAddrToSend = gpLocalAddr[0];

	sprintf(pTmp, "http://%s:80/onvif/device_service", pAddrToSend);
	
	return CopyString(pTmp);
}

char *nativeGetEndpointAddress()
{
	char pTmp[1024]={0};
	char *pAddr = NULL;
	// It is RECOMMENDED that the balue of this element be a stable globally-unique identifier (GUID) base URN[RFC 4122]
	// If the value of this element is not a network-resolvable transport address, 
	// such tansport address(es) are converyed in a separate d:XAddrs element
	
	// "urn:uuid:98190dc2-0890-4ef8-ac9a-5940995e6119" is a example of wsdd-discovery-1.1-spec-cs-01.pdf
	
	//sprintf(pTmp, "urn:uuid:00075f74-9ef6-f69e-745f-%s", getMyMacAddress());
	pAddr = getMyMacAddress();
	sprintf(pTmp, "urn:uuid:98190dc2-0890-4ef8-ac9a-%s", pAddr);	
	free(pAddr);
	return CopyString(pTmp);
}

char *nativeGetTypes()
{
	// For old version, return "dn:NetworkVideoTransmitter"
	
	//{"tds", "http://www.onvif.org/ver10/device/wsdl", NULL, NULL},
	return CopyString("tds:Device");
	
	// {"dn", "http://www.onvif.org/ver10/network/wsdl", NULL, NULL},
	return CopyString("dn:NetworkVideoTransmitter");
}

char *nativeGetScopesItem()
{
	// TODO: the scopes may change, we should reload it every time user invoke this function
	return CopyString("\
onvif://www.onvif.org/type/audio_encoder \
onvif://www.onvif.org/type/video_encoder \
onvif://www.onvif.org/name/albert \
onvif://www.onvif.org/hardware/albertTest \
onvif://www.onvif.org/location/ \
onvif://www.onvif.org/Profile/Streaming");
}

char *nativeGetMessageId()
{
	char pTmp[128]={0}, pTmp2[128]={0};
	UuidGen(pTmp2);
	sprintf(pTmp, "urn:uuid:%s", pTmp2);

	return CopyString(pTmp);	
}

char *nativeGetTo()
{
	// In an ad hoc mode, it MUST be "urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01"
	return CopyString("urn:docs-oasis-open-org:ws-dd:ns:discovery:2009:01");
	
		// If this is a response message
		// CopyString("http://www.w3.org/2006/08/addressing/anonymous");
	
	// In a managed mode, it MUST be the [address] property of the Endpoint Reference of the Discovery Proxy.
}

int nativeGetInstanceId()
{
	return _gInstanceId++;
}

int nativeGetMetadataVersion()
{
	return _gMetadataVersion;
}

void nativeIncreaseMetadataVersion()
{
	// This value will change when receive notify message	
	_gMetadataVersion++;
}

int nativeGetDiscoveryMode()
{
   return _gDiscoveryMode;
}

void nativeChangeDiscoveryMode(char Mode)
{
   if(Mode=='0')
   {
      printf("NONDISCOVERABLE !! \n");
      _gDiscoveryMode = NONDISCOVERABLE;
   }
   else
   {
      printf("DISCOVERABLE !! \n");
      _gDiscoveryMode = DISCOVERABLE;
   }
}

