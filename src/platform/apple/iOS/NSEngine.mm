//
//  NSEngine.mm
//  GowEngineDemo
//
//  Created by Stephen Gowen on 4/23/20.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#import "NSEngine.hpp"

#include "MainEngineController.hpp"
#include "Engine.hpp"

@implementation NSEngine
{
@private
    MainEngineController _controller;
    Engine* _engine;
}

- (NSEngine*)init
{
    _engine = new Engine(_controller);
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

- (int)update:(double)deltaTime
{
    return _engine->update(deltaTime);
}

- (void)render
{
    _engine->render();
}

- (void)onCursorDown:(float)x :(float)y
{
    _engine->onCursorDown(x, y);
}

- (void)onCursorDragged:(float)x :(float)y
{
    _engine->onCursorDragged(x, y);
}

- (void)onCursorUp:(float)x :(float)y
{
    _engine->onCursorUp(x, y);
}

@end
