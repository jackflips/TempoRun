//
//  ViewController.m
//  TempoRun
//
//  Created by John Rogers on 8/2/14.
//  Copyright (c) 2014 metacupcake. All rights reserved.
//

#import "ViewController.h"
#import "ENAPI.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    [ENAPIRequest setApiKey:@"JZSRGTDREFDFN0LRB"];
    
    _audioPlayer = [[AVAudioPlayer alloc] init];
	// Do any additional setup after loading the view, typically from a nib.
}

- (void)playURL:(NSURL*)url {
    self.audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:nil];
    [_audioPlayer setNumberOfLoops:-1];
    [_audioPlayer setMeteringEnabled:YES];
    _audioPlayer.enableRate = YES;
    _audioPlayer.rate = 1.0;
    [_audioPlayer play];
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
                   NSLog(@"%@", [[[[[dict objectForKey:@"response"] objectForKey:@"songs"] objectAtIndex:0] objectForKey:@"audio_summary"] objectForKey:@"tempo"]);
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

