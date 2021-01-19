//
//  NSEngine.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 4/23/20.
//  Copyright © 2020 Stephen Gowen. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSEngine : NSObject

- (instancetype)init:(int)framesPerSecond;

- (void)createDeviceDependentResources;
- (void)onWindowSizeChanged:(int)screenWidth :(int)screenHeight :(int)cursorWidth :(int)cursorHeight;
- (void)releaseDeviceDependentResources;
- (void)onResume;
- (void)onPause;
- (void)update:(double)deltaTime;
- (void)render;
- (void)onCursorDown:(float)x :(float)y :(bool)isAlt;
- (void)onCursorDragged:(float)x :(float)y :(bool)isAlt;
- (void)onCursorUp:(float)x :(float)y :(bool)isAlt;

@end
