//
//  wsdiscovery_api.h
//  WSDiscovery
//
//  Created by albert on 2015/11/3.
//  Copyright (c) 2015年 albert. All rights reserved.
//

#ifndef WSDiscovery_wsdiscovery_api_h
#define WSDiscovery_wsdiscovery_api_h

#import "Notifications.h"

@interface WSDiscovery_API : NSObject

@property NSString *pSoapResponse;
@property NSString *pDeviceIP;

- (int) InitServer:(NSString *)pCmd;
- (int) InitClient:(NSString *)pCommand1 OptionCmd:(NSString *)pCommand2;

- (void) send_msg_update_scopes;
- (void) send_msg_reboot;
- (void) send_msg_discoverymode:(NSInteger) vMode;


@end


@interface WSDiscoveryDevice : NSObject <NSXMLParserDelegate>
{
    NSString *pDeviceName;
    NSString *pWSDiscoveryResponse;
}

- (void) setDeviceName: (NSString *) pName;
- (void) setWSDiscoveryResponse: (NSString *) pResponse;

- (NSString *) getDeviceName;
- (NSString *) getWSDiscoveryResponse;

@end

#endif
