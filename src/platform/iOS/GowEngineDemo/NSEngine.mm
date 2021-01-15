//
//  NSEngine.mm
//  GowEngineDemo
//
//  Created by Stephen Gowen on 4/23/20.
//  Copyright © 2020 Stephen Gowen. All rights reserved.
//

#import <memory>

#import "NSEngine.hpp"
#import "Engine.hpp"

@implementation NSEngine
{
@private
    Engine* _engine;
}

- (NSEngine*)init:(int)framesPerSecond
{
    _engine = new Engine(framesPerSecond);
    return self;
}

- (void)dealloc
{
    delete _engine;
}

- (void)createDeviceDependentResources
{
    _engine->createDeviceDependentResources();
}

- (void)onWindowSizeChanged:(int)screenWidth :(int)screenHeight :(int)cursorWidth :(int)cursorHeight
{
    _engine->onWindowSizeChanged(screenWidth, screenHeight, cursorWidth, cursorHeight);
}

- (void)releaseDeviceDependentResources
{
    _engine->releaseDeviceDependentResources();
}

- (void)onResume
{
    _engine->onResume();
}

- (void)onPause
{
    _engine->onPause();
}

- (void)update:(double)deltaTime
{
    _engine->update(deltaTime);
}

- (void)render
{
    _engine->render();
}

- (void)onCursorDown:(float)x :(float)y :(bool)isAlt
{
    _engine->onCursorDown(x, y, isAlt);
}

- (void)onCursorDragged:(float)x :(float)y :(bool)isAlt
{
    _engine->onCursorDragged(x, y, isAlt);
}

- (void)onCursorUp:(float)x :(float)y :(bool)isAlt
{
    _engine->onCursorUp(x, y, isAlt);
}

@end
