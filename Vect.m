//
//  Vect.m
//  TempoRun
//
//  Created by John Rogers on 8/3/14.
//  Copyright (c) 2014 metacupcake. All rights reserved.
//

#import "Vect.h"

@implementation Vect

- (id)initWithX:(int)x Y:(int)y Z:(int)z {
    if ( self = [super init] ) {
        _x = x;
        _y = y;
        _z = z;
        return self;
    } else {
        return nil;
    }
}

@end
