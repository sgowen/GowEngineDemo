//
//  MainEngineController.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "EngineController.hpp"

class MainEngineController : public EngineController
{
public:
    MainEngineController();
    virtual ~MainEngineController();
    
    virtual EngineState* getInitialState();
};
