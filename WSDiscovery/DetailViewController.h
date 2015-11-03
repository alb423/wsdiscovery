//
//  DetailViewController.h
//  WSDiscovery
//
//  Created by albert on 2015/11/2.
//  Copyright (c) 2015年 albert. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DetailViewController : UIViewController
{
}
@property NSString *pDisplayText;
@property (strong, nonatomic) id detailItem;
@property (strong, nonatomic) IBOutlet UITextView *detailDescriptionTextView;
- (IBAction)DetailView_ScanDevice:(id)sender;
- (void)DetailView_SetDisplayText:(NSString *) pText;
@end

