//
//  MainEngineController.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/15/17.
//  Copyright © 2020 Stephen Gowen. All rights reserved.
//

#include "MainEngineController.hpp"

#include "MainEngineState.hpp"

MainEngineController::MainEngineController()
{
    // TODO
}

MainEngineController::~MainEngineController()
{
    // TODO
}

EngineState* MainEngineController::getInitialState()
{
    return MainEngineState::getInstance();
}
