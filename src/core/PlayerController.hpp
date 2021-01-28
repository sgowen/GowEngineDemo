//
//  PlayerController.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/5/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "EntityController.hpp"

#include <string>

class InputState;

class PlayerController : public EntityController
{
    friend class PlayerNetworkController;
    
    DECL_RTTI;
    DECL_EntityController_create;
    
public:
    PlayerController(Entity* entity);
    virtual ~PlayerController();
    
    virtual void update();
    virtual void receiveMessage(uint16_t message, void* data = NULL);
    virtual void onFixturesCreated(std::vector<b2Fixture*>& fixtures);
    virtual bool shouldCollide(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB);
    virtual void handleBeginContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB);
    virtual void handleEndContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB);
    
    void processInput(InputState* inInputState);
    
    /// Helpers
    void setAddressHash(uint64_t inValue);
    uint64_t getAddressHash() const;
    void setPlayerID(uint8_t inValue);
    uint8_t getPlayerID() const;
    void setMap(uint32_t inValue);
    uint32_t getMap() const;
    void setPlayerName(std::string inValue);
    std::string& getPlayerName();
    
private:
    enum State
    {
        State_IDle = 0,
        State_FirstPunch,
        State_SecondPunch,
        State_ThirdPunch,
        State_Running,
        State_Jumping,
        State_JumpCompleted
    };
    
    enum MainActionFlags
    {
        MainActionFlag_ReadyForSecondPunch = 1,
        MainActionFlag_ToThrowSecondPunch,
        MainActionFlag_ReadyForThirdPunch,
        MainActionFlag_ToThrowThirdPunch
    };
    
    enum ReadStateFlag
    {
        ReadStateFlag_PlayerInfo = 1 << 2,
        ReadStateFlag_Stats =      1 << 3
    };
    
    struct PlayerInfo
    {
        uint64_t _addressHash;
        uint8_t _playerID;
        std::string _playerName;
        
        PlayerInfo()
        {
            _addressHash = 0;
            _playerID = 0;
            _playerName = "Robot";
        }
        
        friend bool operator==(PlayerInfo& lhs, PlayerInfo& rhs)
        {
            return
            lhs._addressHash == rhs._addressHash &&
            lhs._playerID    == rhs._playerID &&
            lhs._playerName  == rhs._playerName;
        }
        
        friend bool operator!=(PlayerInfo& lhs, PlayerInfo& rhs)
        {
            return !(lhs == rhs);
        }
    };
    PlayerInfo _playerInfo;
    PlayerInfo _playerInfoNetworkCache;
    
    struct Stats
    {
        uint8_t _health;
        uint32_t _target;
        
        Stats()
        {
            _health = 3;
            _target = 0;
        }
        
        friend bool operator==(Stats& lhs, Stats& rhs)
        {
            return
            lhs._health == rhs._health &&
            lhs._target == rhs._target;
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
    
    void processInputForIDleState(uint8_t inputState);
    void processInputForFirstPunchState(uint8_t inputState);
    void processInputForSecondPunchState(uint8_t inputState);
    void processInputForThirdPunchState(uint8_t inputState);
    void processInputForRunningState(uint8_t inputState);
    void processInputForJumpingState(uint8_t inputState);
    void processInputForFirstJumpCompletedState(uint8_t inputState);
    
    void handleMovementInput(uint8_t inputState);
    void handleMainActionInput(uint8_t inputState);
    void handleJumpInput(uint8_t inputState);
    void handleJumpCompletedInput(uint8_t inputState);
};

#include "EntityNetworkController.hpp"

class PlayerNetworkController : public EntityNetworkController
{
    DECL_EntityNetworkController_create;
    
public:
    PlayerNetworkController(Entity* e, bool isServer);
    virtual ~PlayerNetworkController();
    
    virtual void read(InputMemoryBitStream& ip);
    virtual uint16_t write(OutputMemoryBitStream& op, uint16_t dirtyState);
    
    virtual void recallNetworkCache();
    virtual uint16_t getDirtyState();
    
    bool isLocalPlayer();
    
private:
    PlayerController* _controller;
    bool _isLocalPlayer;
};
