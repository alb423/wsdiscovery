//
//  DetailViewController.m
//  WSDiscovery
//
//  Created by albert on 2015/11/2.
//  Copyright (c) 2015年 albert. All rights reserved.
//
#import "MasterViewController.h"
#import "DetailViewController.h"
#import "MyUtilities.h"
#import "Reachability.h"
#import "wsdiscovery_api.h"
@interface DetailViewController ()
{
    
}
@end

@implementation DetailViewController
{
    WSDiscovery_API *discoveryAgent;
    NSString        *StringForDetailView;
}
@synthesize pDisplayText;


#pragma mark - Managing the detail item

- (void)setDetailItem:(id)newDetailItem {
    if (_detailItem != newDetailItem) {
        _detailItem = newDetailItem;
            
        // Update the view.
        [self configureView];
    }
}

- (void)configureView {
    // Update the user interface for the detail item.
    if (self.pDisplayText) {
        [self.detailDescriptionTextView setText:pDisplayText];
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view, typically from a nib.
    [self configureView];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)DetailView_SetDisplayText:(NSString *) pText {
    [self.detailDescriptionTextView setText:pText];
}


// This is just a simple demo process
- (IBAction)DetailView_ScanDevice:(id)sender {
    
    // Init wsdiscovery server
    int vRet = 0;
    discoveryAgent = [[WSDiscovery_API alloc] init];
    
    // close the discovery process after 10 seconds
    dispatch_async(GLOBAL_QUEUE, ^(void) {
        sleep(10);
        [discoveryAgent InitClient:@"q" OptionCmd:nil];
    });   
    
    // start discovery to search device
    vRet = [discoveryAgent InitServer:nil];
    
}

@end
