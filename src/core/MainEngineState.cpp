//
//  MainEngineState.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2020 Stephen Gowen. All rights reserved.
//

#include "MainEngineState.hpp"

#include <stdlib.h>
#include <assert.h>

MainEngineState* MainEngineState::s_instance = NULL;

void MainEngineState::create()
{
    assert(!s_instance);
    
    s_instance = new MainEngineState();
}

MainEngineState * MainEngineState::getInstance()
{
    return s_instance;
}

void MainEngineState::destroy()
{
    assert(s_instance);
    
    delete s_instance;
    s_instance = NULL;
}

void MainEngineState::enter(Engine* engine)
{
    // TODO
}

void MainEngineState::execute(Engine* engine)
{
    // TODO
}

void MainEngineState::exit(Engine* engine)
{
    // TODO
}

void MainEngineState::createDeviceDependentResources()
{
    // TODO
}

void MainEngineState::onWindowSizeChanged(int screenWidth, int screenHeight, int cursorWidth, int cursorHeight)
{
    // TODO
}

void MainEngineState::releaseDeviceDependentResources()
{
    // TODO
}

void MainEngineState::onResume()
{
    // Empty
}

void MainEngineState::onPause()
{
    // Empty
}

void MainEngineState::render(double alpha)
{
    // TODO
}

MainEngineState::MainEngineState() : EngineState()
{
    // TODO
}

MainEngineState::~MainEngineState()
{
    // TODO
}
