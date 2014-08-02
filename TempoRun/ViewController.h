//
//  ViewController.h
//  TempoRun
//
//  Created by John Rogers on 8/2/14.
//  Copyright (c) 2014 metacupcake. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>
#import <AVFoundation/AVFoundation.h>

@interface ViewController : UIViewController <MPMediaPickerControllerDelegate>

- (IBAction)pickSong:(id)sender;

@property (strong, nonatomic) AVAudioPlayer *audioPlayer;

@end
