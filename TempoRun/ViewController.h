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
#import <CoreMotion/CoreMotion.h>
#import "Vect.h"
#import <PebbleKit/PebbleKit.h>

@interface ViewController : UIViewController <MPMediaPickerControllerDelegate> {
    int counter;
    int counter2;
    int magCounter;
    float runningVal;
    float periodLargest;
    float periodMean;
    float totalmean;
    NSMutableArray *periodArray;
    double lastTime;
    double tempoOfSong;
    NSURL *currentURL;
    BOOL canChange;
    
    //////REWRITE///////
    NSMutableArray *values;
    Vect *lastVect;
    float queueMean;
    BOOL changeLocked;
    int reportCounter;
    NSMutableArray *buffer;
}

- (IBAction)pickSong:(id)sender;

@property (strong, nonatomic) AVAudioPlayer *audioPlayer;
@property (strong, nonatomic) CMMotionManager *motionManager;
@property (strong, nonatomic) NSMutableArray *queue;
@property (strong, nonatomic) PBWatch *watch;

@end
