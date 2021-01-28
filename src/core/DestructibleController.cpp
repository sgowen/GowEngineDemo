//
//  DestructibleController.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/2/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "DestructibleController.hpp"

#include "Entity.hpp"
#include <box2d/box2d.h>

#include "World.hpp"
#include "Macros.hpp"
#include "Constants.hpp"
#include "Timing.hpp"
#include "StringUtil.hpp"
#include "MathUtil.hpp"
#include "NetworkManagerServer.hpp"
#include "GameInputManager.hpp"
#include "GowAudioEngine.hpp"
#include "Util.hpp"
#include "Config.hpp"
#include "GameEngineState.hpp"

IMPL_RTTI(DestructibleController, EntityController);

IMPL_EntityController_create(DestructibleController);

DestructibleController::DestructibleController(Entity* e) : EntityController(e),
_stats(),
_statsNetworkCache(_stats)
{
    // Empty
}

DestructibleController::~DestructibleController()
{
    // Empty
}

void DestructibleController::update()
{
    if (_entity->getNetworkController()->isServer())
    {
        uint8_t state = _entity->state()._state;
        switch (state)
        {
            case State_Undamaged:
                // Empty
                break;
            case State_OneThirdDamaged:
                // Empty
                break;
            case State_TwoThirdsDamaged:
                // Empty
                break;
            case State_Destructing:
                if (_entity->state()._stateTime >= 180)
                {
                    _entity->requestDeletion();
                }
                break;
            default:
                break;
        }
    }
}

void DestructibleController::receiveMessage(uint16_t message, void* data)
{
    uint8_t fromState = _entity->state()._state;
    uint8_t& state = _entity->state()._state;
    uint16_t& stateTime = _entity->state()._stateTime;
    
    switch (message)
    {
        case ENTITY_MESSAGE_DAMAGE:
        {
            uint8_t& health = _stats._health;
            
            uint32_t* damageP = static_cast<uint32_t*>(data);
            uint32_t& damage = *damageP;
            damage = clamp(damage, 0, health);
            health -= damage;
            if (health == 2 && state != State_OneThirdDamaged)
            {
                state = State_OneThirdDamaged;
                stateTime = 0;
            }
            else if (health == 1 && state != State_TwoThirdsDamaged)
            {
                state = State_TwoThirdsDamaged;
                stateTime = 0;
            }
            else if (health == 0 && state != State_Destructing)
            {
                state = State_Destructing;
                stateTime = 0;
            }
        }
            break;
        default:
            break;
    }
    
    if (GameEngineState::getInstance()->isLive())
    {
        Util::handleSound(_entity, fromState, state);
    }
}

void DestructibleController::onFixturesCreated(std::vector<b2Fixture*>& fixtures)
{
    // Empty
}

bool DestructibleController::shouldCollide(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB)
{
    return _entity->state()._state != State_Destructing;
}

void DestructibleController::handleBeginContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB)
{
    // Empty
}

void DestructibleController::handleEndContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB)
{
    // Empty
}

#include "InputMemoryBitStream.hpp"
#include "OutputMemoryBitStream.hpp"

IMPL_EntityNetworkController_create(DestructibleNetworkController);

DestructibleNetworkController::DestructibleNetworkController(Entity* e, bool isServer) : EntityNetworkController(e, isServer), _controller(static_cast<DestructibleController*>(e->getController()))
{
    // Empty
}

DestructibleNetworkController::~DestructibleNetworkController()
{
    // Empty
}

void DestructibleNetworkController::read(InputMemoryBitStream& ip)
{
    EntityNetworkController::read(ip);
    
    DestructibleController& c = *_controller;
    
    bool stateBit;
    
    ip.read(stateBit);
    if (stateBit)
    {
        ip.read(c._stats._health);
        
        c._statsNetworkCache = c._stats;
    }
}

uint16_t DestructibleNetworkController::write(OutputMemoryBitStream& op, uint16_t dirtyState)
{
    uint16_t writtenState = EntityNetworkController::write(op, dirtyState);
    
    DestructibleController& c = *_controller;
    
    bool stats = dirtyState & DestructibleController::ReadStateFlag_Stats;
    op.write(stats);
    if (stats)
    {
        op.write(c._stats._health);
        
        writtenState |= DestructibleController::ReadStateFlag_Stats;
    }
    
    return writtenState;
}

void DestructibleNetworkController::recallNetworkCache()
{
    EntityNetworkController::recallNetworkCache();
    
    DestructibleController& c = *_controller;
    
    c._stats = c._statsNetworkCache;
}

uint16_t DestructibleNetworkController::getDirtyState()
{
    uint16_t ret = EntityNetworkController::getDirtyState();
    
    DestructibleController& c = *_controller;
    
    if (c._statsNetworkCache != c._stats)
    {
        c._statsNetworkCache = c._stats;
        ret |= DestructibleController::ReadStateFlag_Stats;
    }
    
    return ret;
}
