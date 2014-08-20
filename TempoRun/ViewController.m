//
//  ViewController.m
//  TempoRun
//
//  Created by John Rogers on 8/2/14.
//  Copyright (c) 2014 metacupcake. All rights reserved.
//

#import "ViewController.h"
#import "ENAPI.h"

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    [ENAPIRequest setApiKey:@"JZSRGTDREFDFN0LRB"];
    
    _audioPlayer = [[AVAudioPlayer alloc] init];
    lastVect = nil;
    changeLocked = YES;
    queueMean = 0;
    _queue = [NSMutableArray array];
    counter = 1;
    counter2 = 1;
    reportCounter = 0;
    magCounter = 0;
    runningVal = 0;
    //stddev = 1;
    //largest = 100000;
    periodArray = [NSMutableArray array];
    values = [NSMutableArray array];
    
    _watch = [[PBPebbleCentral
               defaultCentral] lastConnectedWatch];
    
    uuid_t myAppUUIDbytes;
    NSUUID *myAppUUID = [[NSUUID alloc] initWithUUIDString:@"ed943cee-19fe-4913-8110-be59f3787f8b"];
    [myAppUUID getUUIDBytes:myAppUUIDbytes];
    
    [[PBPebbleCentral defaultCentral] setAppUUID:[NSData dataWithBytes:myAppUUIDbytes length:16]];
    
    [_watch appMessagesLaunch:^(PBWatch *watch, NSError *error) {
        if (!error) {
            NSLog(@"Successfully launched app.");
        }
        else {
            NSLog(@"Error launching app - Error: %@", error);
        }
    }
     ];
    
    buffer = [NSMutableArray array];
    
    [_watch appMessagesAddReceiveUpdateHandler:^BOOL(PBWatch *watch, NSDictionary *update) {
        BOOL full = NO;
        if (reportCounter > 10) {
            full = YES;
        }
        
        [buffer removeAllObjects];
        int parseCounter = 0;
        NSString *data = [update objectForKey:[NSNumber numberWithInt:0]];
        Vect *newVect = [[Vect alloc] init];
        while (data.length > 1) {
            int commaLoc = (int)[data rangeOfString:@","].location;
            if (parseCounter == 0) {
                newVect.x = [[data substringToIndex:commaLoc] intValue];
            } else if (parseCounter == 1) {
                newVect.y = [[data substringToIndex:commaLoc] intValue];
            } else if (parseCounter == 2) {
                newVect.z = [[data substringToIndex:commaLoc] intValue];
            } else if (parseCounter == 3) {
                newVect.timeOf = [[data substringToIndex:commaLoc] doubleValue];
                if (!lastVect) {
                    lastVect = newVect;
                } else {
                    Vect *currentVect = [self subtractVect:lastVect other:newVect];
                    NSLog(@"%@", currentVect);
                    currentVect.magnitude = sqrt(pow(currentVect.x, 2) + pow(currentVect.y, 2) + pow(currentVect.z, 2));
                    currentVect.timeOf = newVect.timeOf;
                    [buffer addObject:currentVect];
                    newVect = [[Vect alloc] init];
                    data = [data substringFromIndex:commaLoc+1];
                    parseCounter = 0;
                    continue;
                }
            }
            data = [data substringFromIndex:commaLoc+1];
            parseCounter++;
            
        }
        
        [values addObjectsFromArray:buffer];
        
        NSLog(@"%@", update);
        
        
        /* Now do analysis on the queue */
        
        float stddev = [self sample:values];
        for (Vect *vector in buffer) {
            if (vector.magnitude > queueMean + (stddev * 1.2)) {
                if (magCounter > 4) {
                    //NSLog(@"worked");
                    if (lastTime == 0) {
                        lastTime = vector.timeOf;
                    } else {
                        double period = vector.timeOf - lastTime;
                        [periodArray insertObject:[NSNumber numberWithDouble:period] atIndex:0];
                        lastTime = vector.timeOf;
                        magCounter = 0;
                    }
                    if (reportCounter > 30) {
                        [periodArray removeLastObject];
                    }
                }
                magCounter++;
            }
        }
        
        NSLog(@"%@", periodArray);
        
        /* Now anaylze these values to see if we have any spikes */
        
        if (!changeLocked) {
            NSArray *sortedArray = [periodArray sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
                if ([obj1 doubleValue] > [obj2 doubleValue])
                    return NSOrderedDescending;
                else if ([obj1 floatValue] < [obj2 floatValue])
                    return NSOrderedAscending;
                return NSOrderedSame;
            }];
            double medianPeriod = [[sortedArray objectAtIndex:(int)(sortedArray.count/2)] doubleValue];
            double bpm = (1.0 / medianPeriod) * 60;
            NSLog(@"%f", bpm);
        }
        
        
        
        //[self playURLWithRate:[self pickCorrectTempo:tempoOfSong feetBPM:shit]];
        
        
        
        reportCounter++;
        return YES;
    }];

    /*
    _motionManager = [[CMMotionManager alloc] init];
    [_motionManager startDeviceMotionUpdatesToQueue:[NSOperationQueue currentQueue]
                                            withHandler:^(CMDeviceMotion *motion, NSError *error) {
                                                CMAcceleration accel = motion.userAcceleration;
                                                Vect *current = [[Vect alloc] init];
                                                current.x = accel.x;
                                                current.y = accel.y;
                                                current.z = accel.z;
                                                if (!_lastVect) {
                                                    _lastVect = [[Vect alloc] init];
                                                    _lastVect = current;
                                                    return;
                                                } else {
                                                    Vect *v = [self subtractVect:_lastVect other:current];
                                                    float mag = sqrtf((pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2)));
                                                    if (counter % 4 == 0) {
                                                        [_queue insertObject:[NSNumber numberWithFloat:mag] atIndex:0];
                                                        if (mag > totalmean + (stddev * 1.2)) {
                                                            if (magCounter > 25) {
                                                                double time;
                                                                if (!lastTime) {
                                                                    lastTime = [[NSDate date] timeIntervalSince1970];
                                                                } else {
                                                                    time = [[NSDate date] timeIntervalSince1970] - lastTime;
                                                                    double currentTime =[[NSDate date] timeIntervalSince1970];
                                                                    NSLog(@"%f", time);
                                                                    [periodArray insertObject:[NSNumber numberWithDouble:time] atIndex:0];
                                                                    lastTime = currentTime;
                                                                    //NSLog(@"%f", time);
                                                                    magCounter = 0;
                                                                     if (counter > 1000) {
                                                                         [periodArray removeLastObject];
                                                                     }
                                                                }
                                                            }
                                                        }
                                                        if (_queue.count >= 400
                                                            ) {
                                                            [_queue removeLastObject];
                                                        }
                                                    } else {
                                                        runningVal += mag;
                                                        counter2++;
                                                    }
                                                }
                                                _lastVect = current;
                                                
                                                
                                                if ((counter % 200) == 0) {
                                                    stddev = [self sample:_queue];
                                                    if (canChange) {
                                                        NSArray *sortedArray = [periodArray sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
                                                            if ([obj1 doubleValue] > [obj2 doubleValue])
                                                                return NSOrderedDescending;
                                                            else if ([obj1 floatValue] < [obj2 floatValue])
                                                                return NSOrderedAscending;
                                                            return NSOrderedSame;
                                                        }];
                                                        double fuckyou = [[sortedArray objectAtIndex:(int)(sortedArray.count/2)] doubleValue];
                                                        double shit = (1.0 / fuckyou) * 60;
                                                        
                                                        [self playURLWithRate:[self pickCorrectTempo:tempoOfSong feetBPM:shit]];
                                                    }
                                                }
                                                
                                                if ((counter % 1000) == 0) {
                                                    canChange = YES;
                                                    NSArray *sortedArray = [periodArray sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
                                                        if ([obj1 doubleValue] > [obj2 doubleValue])
                                                            return NSOrderedDescending;
                                                        else if ([obj1 floatValue] < [obj2 floatValue])
                                                            return NSOrderedAscending;
                                                        return NSOrderedSame;
                                                    }];
                                                    double fuckyou = [[sortedArray objectAtIndex:(int)(sortedArray.count/2)] doubleValue];
                                                    double shit = (1.0 / fuckyou) * 60;
                                                    
                                                    [self playURLWithRate:[self pickCorrectTempo:tempoOfSong feetBPM:shit]];
                                                }
                                                counter++;
                                                magCounter++;
                                            }];
     */
    
	// Do any additional setup after loading the view, typically from a nib.
}

- (double)pickCorrectTempo:(double)tempoOfSong feetBPM:(double)feetBPM {
    NSLog(@"tempo: %f, feetbpm: %f", tempoOfSong, feetBPM);
    double modifiedFeetBPM = feetBPM;
    while (abs(modifiedFeetBPM - tempoOfSong) > feetBPM) {
        modifiedFeetBPM += feetBPM;
    }
    return modifiedFeetBPM / tempoOfSong;
}

- (float)sample:(NSMutableArray*)data {
    float mean = 0;
    float largest = 0;
    for (Vect *vector in data) {
        if (vector.magnitude > largest) {
            largest = vector.magnitude;
        }
        mean += vector.magnitude;
    }
    mean = mean / data.count;
    queueMean = mean;
    float workingDev = 0;
    for (Vect *vector in data) {
        workingDev += pow(vector.magnitude - mean, 2);
    }
    workingDev = workingDev / data.count;
    return sqrtf(workingDev);
}


- (float)samplePeriod:(NSMutableArray*)data {
    float mean = 0;
    periodLargest = 0;
    for (NSNumber *pt in data) {
        if (pt.floatValue > periodLargest) periodLargest = pt.floatValue;
        mean += pt.floatValue;
    }
    mean = mean / data.count;
    periodMean = mean;
    float workingDev = 0;
    for (NSNumber *pt in data) {
        workingDev += pow(pt.floatValue - mean, 2);
    }
    workingDev = workingDev / data.count;
    return sqrtf(workingDev);
}

- (Vect*)subtractVect:(Vect*)v1 other:(Vect*)v2 {
    Vect *newVect = [[Vect alloc] init];
    newVect.x = v2.x - v1.x;
    newVect.y = v2.y - v1.y;
    newVect.z = v2.z - v1.z;
    return newVect;
}



- (void)playURL:(NSURL*)url {
    currentURL = url;
    self.audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:nil];
    [_audioPlayer setNumberOfLoops:-1];
    [_audioPlayer setMeteringEnabled:YES];
    _audioPlayer.enableRate = YES;
    _audioPlayer.rate = 1.0;
    [_audioPlayer play];
}

- (void)playURLWithRate:(double)rate {
    NSLog(@"new rate: !!%f", rate);

    _audioPlayer.rate = rate;
}

- (IBAction)pickSong:(id)sender {
    MPMediaPickerController *picker = [[MPMediaPickerController alloc] initWithMediaTypes:MPMediaTypeAnyAudio];
    [picker setDelegate:self];
    [picker setAllowsPickingMultipleItems: NO];
    [self presentViewController:picker animated:YES completion:NULL];
}

- (void)mediaPicker:(MPMediaPickerController *) mediaPicker didPickMediaItems:(MPMediaItemCollection *) collection {
    
    // remove the media picker screen
    [self dismissViewControllerAnimated:YES completion:NULL];
    
    // grab the first selection (media picker is capable of returning more than one selected item,
    // but this app only deals with one song at a time)
    MPMediaItem *item = [[collection items] objectAtIndex:0];
    NSString *title = [item valueForProperty:MPMediaItemPropertyTitle];
    NSString *artistName = [item valueForProperty:MPMediaItemPropertyArtist];
    NSLog(@"title: %@", title);
    NSLog(@"artist: %@", artistName);

    
    // get a URL reference to the selected item
    NSURL *url = [item valueForProperty:MPMediaItemPropertyAssetURL];
    
    NSMutableDictionary *parameters = [NSMutableDictionary new];
    [parameters setValue:artistName forKey:@"artist"];
    [parameters setValue:title forKey:@"title"];
    [parameters setValue:@"1" forKey:@"results"];
    [parameters setValue:@"audio_summary" forKey:@"bucket"];
    
    [ENAPIRequest GETWithEndpoint:@"song/search"
                    andParameters:parameters
               andCompletionBlock:^(ENAPIRequest *request) {
                   NSDictionary *dict = request.response;
                   NSLog(@"%@", request.response);
                   tempoOfSong = [[[[[[dict objectForKey:@"response"] objectForKey:@"songs"] objectAtIndex:0] objectForKey:@"audio_summary"] objectForKey:@"tempo"] doubleValue];
               }];
    
    // pass the URL to playURL:, defined earlier in this file
    [self playURL:url];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end

