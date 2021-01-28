//
//  PlayerController.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/5/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "PlayerController.hpp"

#include "Entity.hpp"
#include <box2d/box2d.h>

#include "GameInputState.hpp"

#include "World.hpp"
#include "Macros.hpp"
#include "Constants.hpp"
#include "Timing.hpp"
#include "StringUtil.hpp"
#include "MathUtil.hpp"
#include "NetworkManagerServer.hpp"
#include "NetworkManagerClient.hpp"
#include "GameInputManager.hpp"
#include "GowAudioEngine.hpp"
#include "Util.hpp"
#include "Config.hpp"
#include "MainConfig.hpp"
#include "GameEngineState.hpp"

IMPL_RTTI(PlayerController, EntityController);

IMPL_EntityController_create(PlayerController);

PlayerController::PlayerController(Entity* e) : EntityController(e),
_playerInfo(),
_playerInfoNetworkCache(_playerInfo),
_stats(),
_statsNetworkCache(_stats),
_attackSensorFixture(NULL)
{
    // Empty
}

PlayerController::~PlayerController()
{
    // Empty
}

void PlayerController::update()
{
    uint8_t& state = _entity->state()._state;
    uint16_t& stateTime = _entity->state()._stateTime;
    
    if ((state == State_FirstPunch && stateTime == 1) ||
        (state == State_SecondPunch && stateTime == 1) ||
        (state == State_ThirdPunch && stateTime == 1))
    {
        if (_stats._target > 0)
        {
            EntityManager* entityManager = _entity->getNetworkController()->isServer() ? NW_MGR_SERVER->getEntityManager() : NW_MGR_CLIENT->getEntityManager();
            Entity* e = entityManager->getEntityByID(_stats._target);
            if (e)
            {
                uint32_t damage = state == State_ThirdPunch ? 2 : 1;
                e->getController()->receiveMessage(ENTITY_MESSAGE_DAMAGE, &damage);
            }
        }
    }
    
    if (_entity->getNetworkController()->isServer())
    {
        if (_stats._health == 0)
        {
            _entity->requestDeletion();
        }
    }
}

void PlayerController::receiveMessage(uint16_t message, void* data)
{
    switch (message)
    {
        case ENTITY_MESSAGE_DAMAGE:
        {
            uint8_t& health = _stats._health;
            
            uint32_t* damageP = static_cast<uint32_t*>(data);
            uint32_t& damage = *damageP;
            damage = clamp(damage, 0, health);
            health -= damage;
        }
            break;
        default:
            break;
    }
}

void PlayerController::onFixturesCreated(std::vector<b2Fixture*>& fixtures)
{
    assert(fixtures.size() >= 4);
    _attackSensorFixture = fixtures[3];
}

bool PlayerController::shouldCollide(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB)
{
    // Don't collide with other players
    return inEntity->getEntityDef()._key != _entity->getEntityDef()._key;
}

void PlayerController::handleBeginContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB)
{
    if (inFixtureA == _attackSensorFixture && !inFixtureB->IsSensor())
    {
        _stats._target = inEntity->getID();
    }
}

void PlayerController::handleEndContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB)
{
    if (inFixtureA == _attackSensorFixture && !inFixtureB->IsSensor())
    {
        _stats._target = 0;
    }
}

void PlayerController::processInput(InputState* inInputState)
{
    GameInputState* is = static_cast<GameInputState*>(inInputState);
    uint8_t playerID = getPlayerID();
    GameInputState::PlayerInputState* playerInputState = is->getPlayerInputStateForID(playerID);
    if (playerInputState == NULL)
    {
        return;
    }
    
    uint8_t fromState = _entity->state()._state;
    uint8_t& state = _entity->state()._state;
    uint8_t inputState = playerInputState->getInputState();
    switch (state)
    {
        case State_IDle:
            processInputForIDleState(inputState);
            break;
        case State_FirstPunch:
            processInputForFirstPunchState(inputState);
            break;
        case State_SecondPunch:
            processInputForSecondPunchState(inputState);
            break;
        case State_ThirdPunch:
            processInputForThirdPunchState(inputState);
            break;
        case State_Running:
            processInputForRunningState(inputState);
            break;
        case State_Jumping:
            processInputForJumpingState(inputState);
            break;
        case State_JumpCompleted:
            processInputForFirstJumpCompletedState(inputState);
            break;
        default:
            break;
    }
    
    if (GameEngineState::getInstance()->isLive())
    {
        Util::handleSound(_entity, fromState, state);
    }
}

void PlayerController::setAddressHash(uint64_t inValue)
{
    _playerInfo._addressHash = inValue;
}

uint64_t PlayerController::getAddressHash() const
{
    return _playerInfo._addressHash;
}

void PlayerController::setPlayerID(uint8_t inValue)
{
    _playerInfo._playerID = inValue;
}

uint8_t PlayerController::getPlayerID() const
{
    return _playerInfo._playerID;
}

void PlayerController::setPlayerName(std::string inValue)
{
    _playerInfo._playerName = inValue;
}

std::string& PlayerController::getPlayerName()
{
    return _playerInfo._playerName;
}

void PlayerController::processInputForIDleState(uint8_t inputState)
{
    handleMovementInput(inputState);
    
    if (_entity->state()._state == State_IDle ||
        _entity->state()._state == State_Running)
    {
        handleMainActionInput(inputState);
    }
    
    if (_entity->state()._state == State_IDle ||
        _entity->state()._state == State_Running)
    {
        handleJumpInput(inputState);
    }
}

void PlayerController::processInputForFirstPunchState(uint8_t inputState)
{
    handleMainActionInput(inputState);
    
    if (_entity->state()._state == State_IDle)
    {
        handleMovementInput(inputState);
    }
    
    if (_entity->state()._state == State_IDle ||
        _entity->state()._state == State_Running)
    {
        handleJumpInput(inputState);
    }
}

void PlayerController::processInputForSecondPunchState(uint8_t inputState)
{
    handleMainActionInput(inputState);
    
    if (_entity->state()._state == State_IDle)
    {
        handleMovementInput(inputState);
    }
    
    if (_entity->state()._state == State_IDle ||
        _entity->state()._state == State_Running)
    {
        handleJumpInput(inputState);
    }
}

void PlayerController::processInputForThirdPunchState(uint8_t inputState)
{
    handleMainActionInput(inputState);
    
    if (_entity->state()._state == State_IDle)
    {
        handleMovementInput(inputState);
    }
    
    if (_entity->state()._state == State_IDle ||
        _entity->state()._state == State_Running)
    {
        handleJumpInput(inputState);
    }
}

void PlayerController::processInputForRunningState(uint8_t inputState)
{
    handleMovementInput(inputState);
    
    if (_entity->state()._state == State_IDle ||
        _entity->state()._state == State_Running)
    {
        handleMainActionInput(inputState);
    }
    
    if (_entity->state()._state == State_IDle ||
        _entity->state()._state == State_Running)
    {
        handleJumpInput(inputState);
    }
}

void PlayerController::processInputForJumpingState(uint8_t inputState)
{
    handleJumpInput(inputState);
}

void PlayerController::processInputForFirstJumpCompletedState(uint8_t inputState)
{
    handleJumpCompletedInput(inputState);
}

void PlayerController::handleMovementInput(uint8_t inputState)
{
    static const int LEFT = 0;
    static const int STOP = 1;
    static const int RIGHT = 2;
    
    uint8_t& state = _entity->state()._state;
    
    state = State_IDle;
    int moveState = STOP;
    if (_entity->isGrounded())
    {
        if (inputState & GameInputStateFlag_MovingRight)
        {
            state = State_Running;
            moveState = RIGHT;
        }
        else if (inputState & GameInputStateFlag_MovingLeft)
        {
            state = State_Running;
            moveState = LEFT;
        }
    }
    else
    {
        state = State_JumpCompleted;
    }
    
    const b2Vec2& vel = _entity->getVelocity();
    float desiredVel = 0;
    switch (moveState)
    {
        case LEFT:
            desiredVel = b2Max(vel.x - 1, -CFG_MAIN._maxRobotVelocityX);
            break;
        case STOP:
            desiredVel = vel.x * 0.99f;
            break;
        case RIGHT:
            desiredVel = b2Min(vel.x + 1, CFG_MAIN._maxRobotVelocityX);
            break;
    }
    
    float velChange = desiredVel - vel.x;
    float impulse = _entity->getBody()->GetMass() * velChange;
    
    _entity->pose()._isFacingLeft = moveState == LEFT ? true : moveState == RIGHT ? false : _entity->pose()._isFacingLeft;
    _entity->updateBodyFromPose();
    
    if (!areFloatsPracticallyEqual(impulse, 0))
    {
        _entity->getBody()->ApplyLinearImpulse(b2Vec2(impulse, 0), _entity->getBody()->GetWorldCenter(), true);
    }
}

void PlayerController::handleMainActionInput(uint8_t inputState)
{
    uint8_t& state = _entity->state()._state;
    uint16_t& stateTime = _entity->state()._stateTime;
    uint8_t& stateFlags = _entity->state()._stateFlags;
    
    if (inputState & GameInputStateFlag_MainAction && _entity->isGrounded())
    {
        if (stateFlags == 0 && state != State_FirstPunch && state != State_SecondPunch && state != State_ThirdPunch)
        {
            state = State_FirstPunch;
            stateTime = 0;
            stateFlags = 0;
        }
        else if (stateFlags == MainActionFlag_ReadyForSecondPunch && stateTime < 30 && state != State_SecondPunch)
        {
            stateFlags = MainActionFlag_ToThrowSecondPunch;
        }
        else if (stateFlags == MainActionFlag_ReadyForThirdPunch && stateTime < 30 && state != State_ThirdPunch)
        {
            stateFlags = MainActionFlag_ToThrowThirdPunch;
        }
        
        if (stateFlags == MainActionFlag_ToThrowSecondPunch && stateTime > 18)
        {
            state = State_SecondPunch;
            stateTime = 0;
            stateFlags = 0;
        }
        else if (stateFlags == MainActionFlag_ToThrowThirdPunch && stateTime > 18)
        {
            state = State_ThirdPunch;
            stateTime = 0;
            stateFlags = 0;
        }
    }
    else
    {
        if (_entity->isGrounded())
        {
            if (state == State_FirstPunch)
            {
                if (stateFlags == 0)
                {
                    stateFlags = MainActionFlag_ReadyForSecondPunch;
                }
            }
            else if (state == State_SecondPunch)
            {
                if (stateFlags == 0)
                {
                    stateFlags = MainActionFlag_ReadyForThirdPunch;
                }
            }
            else if (state == State_ThirdPunch)
            {
                stateFlags = 0;
            }
        }
        else if (state == State_FirstPunch || state == State_SecondPunch || state == State_ThirdPunch)
        {
            state = State_IDle;
            stateFlags = 0;
        }
        
        if (stateFlags == MainActionFlag_ToThrowSecondPunch && stateTime > 18)
        {
            state = State_SecondPunch;
            stateTime = 0;
            stateFlags = 0;
        }
        else if (stateFlags == MainActionFlag_ToThrowThirdPunch && stateTime > 18)
        {
            state = State_ThirdPunch;
            stateTime = 0;
            stateFlags = 0;
        }
        
        if ((state == State_FirstPunch && stateTime > 30) ||
            (state == State_SecondPunch && stateTime > 30) ||
            (state == State_ThirdPunch && stateTime > 30))
        {
            state = State_IDle;
            stateFlags = 0;
        }
    }
}

void PlayerController::handleJumpInput(uint8_t inputState)
{
    uint8_t& state = _entity->state()._state;
    uint16_t& stateTime = _entity->state()._stateTime;
    
    if (inputState & GameInputStateFlag_Jumping && _entity->isGrounded())
    {
        if (state != State_Jumping)
        {
            _entity->setVelocity(b2Vec2(_entity->getVelocity().x, 0));
            stateTime = 0;
            state = State_Jumping;
        }
        
        float vertForce = 0;
        if (state == State_Jumping)
        {
            vertForce = _entity->getBody()->GetMass() * (CFG_MAIN._maxRobotVelocityY - stateTime * 0.5f);
            vertForce = clamp(vertForce, 0, _entity->getBody()->GetMass() * CFG_MAIN._maxRobotVelocityY);
            _entity->getBody()->ApplyLinearImpulse(b2Vec2(0, vertForce), _entity->getBody()->GetWorldCenter(), true);
        }
        
        if (stateTime > 18 || vertForce <= 0)
        {
            state = State_JumpCompleted;
        }
    }
    else if (state == State_Jumping)
    {
        state = State_JumpCompleted;
    }
}

void PlayerController::handleJumpCompletedInput(uint8_t inputState)
{
    uint8_t& state = _entity->state()._state;
    uint16_t& stateTime = _entity->state()._stateTime;
    
    if (_entity->isGrounded() && stateTime > 18)
    {
        state = State_IDle;
    }
}

#include "InputMemoryBitStream.hpp"
#include "OutputMemoryBitStream.hpp"

IMPL_EntityNetworkController_create(PlayerNetworkController);

PlayerNetworkController::PlayerNetworkController(Entity* e, bool isServer) : EntityNetworkController(e, isServer), _controller(static_cast<PlayerController*>(e->getController())), _isLocalPlayer(false)
{
    // Empty
}

PlayerNetworkController::~PlayerNetworkController()
{
    // Empty
}

void PlayerNetworkController::read(InputMemoryBitStream& ip)
{
    uint8_t fromState = _entity->stateNetworkCache()._state;
    
    EntityNetworkController::read(ip);
    
    PlayerController& c = *_controller;
    
    bool stateBit;
    
    ip.read(stateBit);
    if (stateBit)
    {
        ip.read(c._playerInfo._addressHash);
        ip.read<uint8_t, 3>(c._playerInfo._playerID);
        ip.readSmall(c._playerInfo._playerName);
        
        _isLocalPlayer = NW_MGR_CLIENT->isPlayerIDLocal(c._playerInfo._playerID);
        
        c._playerInfoNetworkCache = c._playerInfo;
    }
    
    ip.read(stateBit);
    if (stateBit)
    {
        ip.read(c._stats._health);
        ip.read(c._stats._target);
        
        c._statsNetworkCache = c._stats;
    }
    
    if (!_isLocalPlayer)
    {
        Util::handleSound(_entity, fromState, _entity->state()._state);
    }
}

uint16_t PlayerNetworkController::write(OutputMemoryBitStream& op, uint16_t dirtyState)
{
    uint16_t writtenState = EntityNetworkController::write(op, dirtyState);
    
    PlayerController& c = *_controller;
    
    bool playerInfo = dirtyState & PlayerController::ReadStateFlag_PlayerInfo;
    op.write(playerInfo);
    if (playerInfo)
    {
        op.write(c._playerInfo._addressHash);
        op.write<uint8_t, 3>(c._playerInfo._playerID);
        op.writeSmall(c._playerInfo._playerName);
        
        writtenState |= PlayerController::ReadStateFlag_PlayerInfo;
    }
    
    bool stats = dirtyState & PlayerController::ReadStateFlag_Stats;
    op.write(stats);
    if (stats)
    {
        op.write(c._stats._health);
        op.write(c._stats._target);
        
        writtenState |= PlayerController::ReadStateFlag_Stats;
    }
    
    return writtenState;
}

void PlayerNetworkController::recallNetworkCache()
{
    EntityNetworkController::recallNetworkCache();
    
    PlayerController& c = *_controller;
    
    c._playerInfo = c._playerInfoNetworkCache;
    c._stats = c._statsNetworkCache;
}

uint16_t PlayerNetworkController::getDirtyState()
{
    uint16_t ret = EntityNetworkController::getDirtyState();
    
    PlayerController& c = *_controller;
    
    if (c._playerInfoNetworkCache != c._playerInfo)
    {
        c._playerInfoNetworkCache = c._playerInfo;
        ret |= PlayerController::ReadStateFlag_PlayerInfo;
    }
    
    if (c._statsNetworkCache != c._stats)
    {
        c._statsNetworkCache = c._stats;
        ret |= PlayerController::ReadStateFlag_Stats;
    }
    
    return ret;
}

bool PlayerNetworkController::isLocalPlayer()
{
    return _isLocalPlayer;
}
