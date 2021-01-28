//
//  MainEngineController.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "MainEngineController.hpp"

#include "Timing.hpp"
#include "EntityIDManager.hpp"
#include "InstanceManager.hpp"
#include "EntityMapper.hpp"
#include "Constants.hpp"

#include "MainEngineState.hpp"
#include "StudioEngineState.hpp"
#include "GameEngineState.hpp"

#include "BasicFollowAndAttackController.hpp"
#include "DestructibleController.hpp"
#include "PlayerController.hpp"

MainEngineController::MainEngineController()
{
    static Timing timingServer;
    static Timing timingClient;
    static EntityIDManager entityIDManagerServer;
    static EntityIDManager entityIDManagerClient;
    static EntityIDManager entityIDManagerStudio;
    
    INSTANCE_MGR.registerInstance(INSTANCE_TIME_SERVER, &timingServer);
    INSTANCE_MGR.registerInstance(INSTANCE_TIME_CLIENT, &timingClient);
    INSTANCE_MGR.registerInstance(INSTANCE_ENTITY_ID_MANAGER_SERVER, &entityIDManagerServer);
    INSTANCE_MGR.registerInstance(INSTANCE_ENTITY_ID_MANAGER_CLIENT, &entityIDManagerClient);
    INSTANCE_MGR.registerInstance(INSTANCE_ENTITY_ID_MANAGER_STUDIO, &entityIDManagerStudio);
    
    ENTITY_MAPPER.registerFunction("BasicFollowAndAttack", BasicFollowAndAttackController::create);
    ENTITY_MAPPER.registerFunction("BasicFollowAndAttack", BasicFollowAndAttackNetworkController::create);
    ENTITY_MAPPER.registerFunction("Destructible", DestructibleController::create);
    ENTITY_MAPPER.registerFunction("Destructible", DestructibleNetworkController::create);
    ENTITY_MAPPER.registerFunction("Player", PlayerController::create);
    ENTITY_MAPPER.registerFunction("Player", PlayerNetworkController::create);
    
    MainEngineState::create();
    GameEngineState::create();
    StudioEngineState::create(GameEngineState::sHandleTest);
}

MainEngineController::~MainEngineController()
{
    MainEngineState::destroy();
    GameEngineState::destroy();
    StudioEngineState::destroy();
}

EngineState* MainEngineController::getInitialState()
{
    return MainEngineState::getInstance();
}
