//
//  Vect.h
//  TempoRun
//
//  Created by John Rogers on 8/3/14.
//  Copyright (c) 2014 metacupcake. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Vect : NSObject

@property (nonatomic) int x;
@property (nonatomic) int y;
@property (nonatomic) int z;
@property (nonatomic) double timeOf;
@property (nonatomic) float magnitude;

- (id)initWithX:(int)x Y:(int)y Z:(int)z;

@end
