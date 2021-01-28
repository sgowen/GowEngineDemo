//
//  BasicFollowAndAttackController.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/5/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "EntityController.hpp"

class BasicFollowAndAttackController : public EntityController
{
    friend class BasicFollowAndAttackNetworkController;
    
    DECL_RTTI;
    DECL_EntityController_create;
    
public:
    BasicFollowAndAttackController(Entity* entity);
    virtual ~BasicFollowAndAttackController();
    
    virtual void update();
    virtual void receiveMessage(uint16_t message, void* data = NULL);
    virtual void onFixturesCreated(std::vector<b2Fixture*>& fixtures);
    virtual bool shouldCollide(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB);
    virtual void handleBeginContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB);
    virtual void handleEndContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB);
    
private:
    enum State
    {
        State_IDle = 0,
        State_Moving,
        State_Attacking,
        State_Dying
    };
    
    enum ReadStateFlag
    {
        ReadStateFlag_Stats = 1 << 2
    };
    
    struct Stats
    {
        uint8_t _health;
        
        Stats()
        {
            _health = 3;
        }
        
        friend bool operator==(Stats& lhs, Stats& rhs)
        {
            return
            lhs._health == rhs._health;
        }
        
        friend bool operator!=(Stats& lhs, Stats& rhs)
        {
            return !(lhs == rhs);
        }
    };
    Stats _stats;
    Stats _statsNetworkCache;
    
    /// Non-Networked
    b2Fixture* _attackSensorFixture;
    
    /// Server Only
    uint32_t _target;
    uint32_t _targetTouching;
    
    void handleIDleState();
    void handleMovingState();
    void handleAttackingState();
    void handleDyingState();
};

#include "EntityNetworkController.hpp"

class BasicFollowAndAttackNetworkController : public EntityNetworkController
{
    DECL_EntityNetworkController_create;
    
public:
    BasicFollowAndAttackNetworkController(Entity* e, bool isServer);
    virtual ~BasicFollowAndAttackNetworkController();
    
    virtual void read(InputMemoryBitStream& ip);
    virtual uint16_t write(OutputMemoryBitStream& op, uint16_t dirtyState);
    
    virtual void recallNetworkCache();
    virtual uint16_t getDirtyState();
    
private:
    BasicFollowAndAttackController* _controller;
};
