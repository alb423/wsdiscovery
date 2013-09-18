#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/file.h>  // for flock

#include "soapH.h"
SOAP_NMAC struct Namespace namespaces[] =
{
   {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
   {"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
   {"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
   {"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
   {"wsrf-bf2", "http://docs.oasis-open.org/wsrf/bf-2", NULL, NULL},
   {"wstop", "http://docs.oasis-open.org/wsn/t-1", NULL, NULL},
   {"wsrf-r", "http://docs.oasis-open.org/wsrf/r-1", NULL, NULL},
   {"wsrf-bf", "http://docs.oasis-open.org/wsrf/bf-1", NULL, NULL},
   {"wsa", "http://schemas.xmlsoap.org/ws/2004/08/addressing", NULL, NULL},
   {"wsa2", "http://www.w3.org/2005/03/addressing", NULL, NULL},
   {"c14n", "http://www.w3.org/2001/10/xml-exc-c14n#", NULL, NULL},
   {"wsu", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd", NULL, NULL},
   {"xenc", "http://www.w3.org/2001/04/xmlenc#", NULL, NULL},
   {"ds", "http://www.w3.org/2000/09/xmldsig#", NULL, NULL},
   {"wsa5", "http://www.w3.org/2005/08/addressing", NULL, NULL},
   {"wsse", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd", NULL, NULL},
   {"wsrf-r2", "http://docs.oasis-open.org/wsrf/r-2", NULL, NULL},
   {"wsnt", "http://docs.oasis-open.org/wsn/b-2", NULL, NULL},
   {NULL, NULL, NULL, NULL}
};

int main(int argc, char **argv)
{
   printf("Hello\n");
}

SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *pSoap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Hello(struct soap *pSoap, struct wsdd__HelloType *wsdd__Hello)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Bye(struct soap *pSoap, struct wsdd__ByeType *wsdd__Bye)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Probe(struct soap *pSoap, struct wsdd__ProbeType *wsdd__Probe)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ProbeMatches(struct soap *pSoap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__Resolve(struct soap *pSoap, struct wsdd__ResolveType *wsdd__Resolve)
{return SOAP_FAULT;}

SOAP_FMAC5 int SOAP_FMAC6 __wsdd__ResolveMatches(struct soap *pSoap, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{return SOAP_FAULT;}
