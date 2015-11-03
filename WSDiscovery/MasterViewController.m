//
//  MasterViewController.m
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


extern int client(char *pCmd1, char *pCmd2);
extern int server(char *pCmd1);
@interface MasterViewController ()

@property NSMutableArray *objects;
@property NSMutableOrderedSet *deviceIPList;
@end

@implementation MasterViewController
{
    NSInteger vDeviceCounter;
}

- (void)awakeFromNib {
    [super awakeFromNib];
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        self.clearsSelectionOnViewWillAppear = NO;
        self.preferredContentSize = CGSizeMake(320.0, 600.0);
    }
}



- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    self.navigationItem.leftBarButtonItem = self.editButtonItem;

    UIBarButtonItem *addButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(insertNewObject:)];
    self.navigationItem.rightBarButtonItem = addButton;
    self.detailViewController = (DetailViewController *)[[self.splitViewController.viewControllers lastObject] topViewController];
    
    vDeviceCounter = 0;

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleNotification:) name:CheckNetworkAvailablityAndReConnect object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleNotification:) name:StartButtonTapped object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleNotification:) name:StopButtonTapped object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleNotification:) name:WSDiscoveryResponse object:nil];

}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (void)insertNewObject:(id)sender {
    WSDiscoveryDevice *pWSDiscoveryDevice ;
    if (!self.objects) {
        self.objects = [[NSMutableArray alloc] init];
    }
    
    if (!self.deviceIPList) {
        self.deviceIPList = [[NSMutableOrderedSet alloc] init];
    }
    

    if(sender!=nil)
    {
        pWSDiscoveryDevice = (WSDiscoveryDevice *)sender;
    }
    else
    {
        pWSDiscoveryDevice = [[WSDiscoveryDevice alloc]init];
    }
    
    NSString *pCheck = [pWSDiscoveryDevice getDeviceName];
    if([self.deviceIPList containsObject:pCheck])
    {
        NSLog(@"duplicate device name: %@", pCheck);
        return;
    }
    else
    {
        [self.deviceIPList addObject:[pWSDiscoveryDevice getDeviceName]];
    }
    
    [self.objects insertObject:pWSDiscoveryDevice atIndex:0];
    
    NSIndexPath *indexPath = [NSIndexPath indexPathForRow:0 inSection:0];
    [self.tableView insertRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
}

#pragma mark - Segues

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"showDetail"]) {
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
        WSDiscoveryDevice *pData = self.objects[indexPath.row];
        
        
        DetailViewController *controller = (DetailViewController *)[[segue destinationViewController] topViewController];
        controller.pDisplayText = [[NSString alloc] initWithFormat:@"%@",[pData getWSDiscoveryResponse]];
        
        controller.navigationItem.leftBarButtonItem = self.splitViewController.displayModeButtonItem;
        controller.navigationItem.leftItemsSupplementBackButton = YES;
    }
}

#pragma mark - Table View

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.objects.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"Cell" forIndexPath:indexPath];
    
    WSDiscoveryDevice *pData = self.objects[indexPath.row];
    cell.textLabel.text = [pData getDeviceName];
    
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        [self.objects removeObjectAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }
}

-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSLog(@"indexPath.row=%ld",(long)indexPath.row);

}

#pragma mark - Remote Handling
/*  This method logs out when a
 *  remote control button is pressed.
 *
 *  In some cases, it will also manipulate the stream.
 */

- (void)handleNotification:(NSNotification *)notification
{
    if ([notification.name isEqualToString:StartButtonTapped]) {
        
    }
    else if ([notification.name isEqualToString:StopButtonTapped]) {
        
    }
    else if ([notification.name isEqualToString:WSDiscoveryResponse]) {
        
        WSDiscovery_API *pTmpDiscoveryInfo = (WSDiscovery_API *)notification.object;
        NSLog(@"%@", pTmpDiscoveryInfo);

        //NSString *pDisplayText = [[NSString alloc]initWithFormat: @"Find %ld devices", (long)vDeviceCounter];
        
        // pTmpDiscoveryInfo.pDeviceIP)
        //if(1)
        {
            WSDiscoveryDevice *pWSDiscoveryDevice = [[WSDiscoveryDevice alloc]init];
            vDeviceCounter++;
            [pWSDiscoveryDevice setDeviceName:pTmpDiscoveryInfo.pDeviceIP];
            [pWSDiscoveryDevice setWSDiscoveryResponse:pTmpDiscoveryInfo.pSoapResponse];

            dispatch_async(MAIN_QUEUE, ^(void) {
                [self insertNewObject:pWSDiscoveryDevice];
            });
        }
    }
    else if ([notification.name isEqualToString:CheckNetworkAvailablityAndReConnect]) {
        
    }
}

@end
