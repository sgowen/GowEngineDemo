//
//  MainEngineController.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "MainEngineController.hpp"

#include "MainEngineState.hpp"

MainEngineController::MainEngineController()
{
    MainEngineState::create();
}

MainEngineController::~MainEngineController()
{
    MainEngineState::destroy();
}

EngineState* MainEngineController::getInitialState()
{
    return MainEngineState::getInstance();
}
