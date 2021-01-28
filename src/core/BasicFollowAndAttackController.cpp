//
//  BasicFollowAndAttackController.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/5/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "BasicFollowAndAttackController.hpp"

#include "Entity.hpp"
#include <box2d/box2d.h>

#include "World.hpp"
#include "Macros.hpp"
#include "Constants.hpp"
#include "Timing.hpp"
#include "StringUtil.hpp"
#include "MathUtil.hpp"
#include "NetworkManagerServer.hpp"
#include "GowAudioEngine.hpp"
#include "EntityManager.hpp"
#include "Util.hpp"
#include "Server.hpp"
#include "MainConfig.hpp"
#include "PlayerController.hpp"
#include "Config.hpp"
#include "GameEngineState.hpp"

IMPL_RTTI(BasicFollowAndAttackController, EntityController);

IMPL_EntityController_create(BasicFollowAndAttackController);

BasicFollowAndAttackController::BasicFollowAndAttackController(Entity* e) : EntityController(e),
_stats(),
_statsNetworkCache(_stats),
_attackSensorFixture(NULL),
_target(0),
_targetTouching(0)
{
    // Empty
}

BasicFollowAndAttackController::~BasicFollowAndAttackController()
{
    // Empty
}

void BasicFollowAndAttackController::update()
{
    if (_entity->getNetworkController()->isServer())
    {
        uint8_t state = _entity->state()._state;
        switch (state)
        {
            case State_IDle:
                handleIDleState();
                break;
            case State_Moving:
                handleMovingState();
                break;
            case State_Attacking:
                handleAttackingState();
                break;
            case State_Dying:
                handleDyingState();
                break;
            default:
                break;
        }
    }
}

void BasicFollowAndAttackController::receiveMessage(uint16_t message, void* data)
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
            if (health == 0 && state != State_Dying)
            {
                state = State_Dying;
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

void BasicFollowAndAttackController::onFixturesCreated(std::vector<b2Fixture*>& fixtures)
{
    assert(fixtures.size() >= 2);
    _attackSensorFixture = fixtures[1];
}

bool BasicFollowAndAttackController::shouldCollide(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB)
{
    // Don't collide with other crawlers
    return inEntity->getEntityDef()._key != _entity->getEntityDef()._key &&
    _entity->state()._state != State_Dying;
}

void BasicFollowAndAttackController::handleBeginContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB)
{
    if (_entity->getNetworkController()->isServer() &&
        inFixtureA == _attackSensorFixture &&
        !inFixtureB->IsSensor() &&
        inEntity->getController()->getRTTI().derivesFrom(PlayerController::rtti))
    {
        _targetTouching = inEntity->getID();
    }
}

void BasicFollowAndAttackController::handleEndContact(Entity* inEntity, b2Fixture* inFixtureA, b2Fixture* inFixtureB)
{
    if (_entity->getNetworkController()->isServer() &&
        inFixtureA == _attackSensorFixture &&
        !inFixtureB->IsSensor() &&
        inEntity->getController()->getRTTI().derivesFrom(PlayerController::rtti))
    {
        _targetTouching = 0;
    }
}

void BasicFollowAndAttackController::handleIDleState()
{
    World* w = Server::getInstance()->getWorld();
    assert(w);
    
    std::vector<Entity*>& players = w->getPlayers();
    
    float shortestDistance = CFG_MAIN._camWidth / 3;
    
    for (Entity* e : players)
    {
        float distance = b2Distance(e->getPosition(), _entity->getPosition());
        if (distance < shortestDistance)
        {
            shortestDistance = distance;
            _target = e->getID();
        }
    }
    
    uint8_t& state = _entity->state()._state;
    uint16_t& stateTime = _entity->state()._stateTime;
    
    if (_target)
    {
        state = State_Moving;
        stateTime = 0;
    }
}

void BasicFollowAndAttackController::handleMovingState()
{
    EntityManager* em = NW_MGR_SERVER->getEntityManager();
    uint8_t& state = _entity->state()._state;
    uint16_t& stateTime = _entity->state()._stateTime;
    if (_targetTouching && em->getEntityByID(_targetTouching))
    {
        state = State_Attacking;
        stateTime = 0;
        return;
    }
    
    Entity* target = em->getEntityByID(_target);
    if (target)
    {
        float distance = b2Distance(target->getPosition(), target->getPosition());
        if (distance > CFG_MAIN._camWidth / 3)
        {
            _target = 0;
            state = State_IDle;
            stateTime = 0;
            
            return;
        }
    }
    else
    {
        _target = 0;
        state = State_IDle;
        stateTime = 0;
        
        return;
    }
    
    bool isRight = target->getPosition().x > _entity->getPosition().x;
    bool isLeft = target->getPosition().x < _entity->getPosition().x;
    
    static const int LEFT = 0;
    static const int STOP = 1;
    static const int RIGHT = 2;
    
    int moveState = STOP;
    if (isRight)
    {
        moveState = RIGHT;
    }
    else if (isLeft)
    {
        moveState = LEFT;
    }
    
    const b2Vec2& vel = _entity->getVelocity();
    float desiredVel = 0;
    switch (moveState)
    {
        case LEFT:
            desiredVel = b2Max(vel.x - 1, -CFG_MAIN._maxCrawlerVelocityX);
            break;
        case STOP:
            desiredVel = vel.x * 0.99f;
            break;
        case RIGHT:
            desiredVel = b2Min(vel.x + 1, CFG_MAIN._maxCrawlerVelocityX);
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

void BasicFollowAndAttackController::handleAttackingState()
{
    uint8_t& state = _entity->state()._state;
    uint16_t& stateTime = _entity->state()._stateTime;
    
    Entity* targetTouching = NW_MGR_SERVER->getEntityManager()->getEntityByID(_targetTouching);
    if (targetTouching)
    {
        uint16_t animTime = stateTime % 30;
        if (animTime == 24)
        {
            uint32_t damage = 1;
            targetTouching->getController()->receiveMessage(ENTITY_MESSAGE_DAMAGE, &damage);
        }
    }
    else
    {
        state = State_IDle;
        stateTime = 0;
    }
}

void BasicFollowAndAttackController::handleDyingState()
{
    if (_entity->state()._stateTime >= 180)
    {
        _entity->requestDeletion();
    }
}

#include "InputMemoryBitStream.hpp"
#include "OutputMemoryBitStream.hpp"

IMPL_EntityNetworkController_create(BasicFollowAndAttackNetworkController);

BasicFollowAndAttackNetworkController::BasicFollowAndAttackNetworkController(Entity* e, bool isServer) : EntityNetworkController(e, isServer), _controller(static_cast<BasicFollowAndAttackController*>(e->getController()))
{
    // Empty
}

BasicFollowAndAttackNetworkController::~BasicFollowAndAttackNetworkController()
{
    // Empty
}

void BasicFollowAndAttackNetworkController::read(InputMemoryBitStream& ip)
{
    uint8_t fromState = _entity->stateNetworkCache()._state;
    
    EntityNetworkController::read(ip);
    
    BasicFollowAndAttackController& c = *_controller;
    
    bool stateBit;
    
    ip.read(stateBit);
    if (stateBit)
    {
        ip.read(c._stats._health);
        
        c._statsNetworkCache = c._stats;
    }
    
    Util::handleSound(_entity, fromState, _entity->state()._state);
}

uint16_t BasicFollowAndAttackNetworkController::write(OutputMemoryBitStream& op, uint16_t dirtyState)
{
    uint16_t writtenState = EntityNetworkController::write(op, dirtyState);
    
    BasicFollowAndAttackController& c = *_controller;
    
    bool stats = dirtyState & BasicFollowAndAttackController::ReadStateFlag_Stats;
    op.write(stats);
    if (stats)
    {
        op.write(c._stats._health);
        
        writtenState |= BasicFollowAndAttackController::ReadStateFlag_Stats;
    }
    
    return writtenState;
}

void BasicFollowAndAttackNetworkController::recallNetworkCache()
{
    EntityNetworkController::recallNetworkCache();
    
    BasicFollowAndAttackController& c = *_controller;
    
    c._stats = c._statsNetworkCache;
}

uint16_t BasicFollowAndAttackNetworkController::getDirtyState()
{
    uint16_t ret = EntityNetworkController::getDirtyState();
    
    BasicFollowAndAttackController& c = *_controller;
    
    if (c._statsNetworkCache != c._stats)
    {
        c._statsNetworkCache = c._stats;
        ret |= BasicFollowAndAttackController::ReadStateFlag_Stats;
    }
    
    return ret;
}
