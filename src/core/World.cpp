//
//  World.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 5/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "World.hpp"

#include "Entity.hpp"
#include "EntityIDManager.hpp"
#include <box2d/box2d.h>

#include "EntityMapper.hpp"
#include "EntityLayoutMapper.hpp"
#include "EntityNetworkController.hpp"
#include "InstanceManager.hpp"
#include "MainConfig.hpp"
#include "Timing.hpp"
#include "StringUtil.hpp"
#include "GowUtil.hpp"

World::World(uint32_t flags) :
_flags(flags),
_world(new b2World(b2Vec2(0.0f, MAIN_CFG._gravity))),
_entityContactListener(new EntityContactListener()),
_entityContactFilter(new EntityContactFilter()),
_entityIDManager(static_cast<EntityIDManager*>(INSTANCE_MGR.get(_flags & WorldFlag_Studio ? INSTANCE_ENTITY_ID_MANAGER_STUDIO : _flags & WorldFlag_Server ? INSTANCE_ENTITY_ID_MANAGER_SERVER : INSTANCE_ENTITY_ID_MANAGER_CLIENT))),
_map()
{
    _world->SetContactListener(_entityContactListener);
    _world->SetContactFilter(_entityContactFilter);
}

World::~World()
{
    clear();
    
    deinitPhysics(_dynamicEntities);
    GowUtil::cleanUpVectorOfPointers(_dynamicEntities);
    refreshPlayers();
    
    delete _entityContactListener;
    delete _entityContactFilter;
    delete _world;
}

void World::loadMap(uint32_t map)
{
    assert(map);
    
    clear();
    
    _map.key = map;
    _map.name = StringUtil::stringFromFourChar(map);
    _map.fileName = ENTITY_LAYOUT_MAPPER.getJsonConfigFilePath(map);
    
    ENTITY_LAYOUT_MAPPER.loadEntityLayout(_map.key, _entityIDManager);
    
    EntityLayoutDef& eld = ENTITY_LAYOUT_MAPPER.getEntityLayoutDef();
    for (EntityInstanceDef eid : eld._entities)
    {
        EntityDef* ed = ENTITY_MAPPER.getEntityDef(eid._type);
        if (_flags & WorldFlag_Client && isDynamic(*ed))
        {
            // On the client, Dynamic Entities must arrive via network
            continue;
        }
        
        Entity* e = ENTITY_MAPPER.createEntity(&eid, _flags & WorldFlag_Server);
        addEntity(e);
    }
}

void World::saveMap()
{
    saveMapAs(_map.key);
}

void World::saveMapAs(uint32_t map)
{
    EntityLayoutDef layout;
    
    for (Entity* e : _layers)
    {
        layout._entities.push_back(EntityInstanceDef(e->getID(), e->getEntityDef().type, e->getPosition().x, e->getPosition().y));
    }
    
    for (Entity* e : _staticEntities)
    {
        layout._entities.push_back(EntityInstanceDef(e->getID(), e->getEntityDef().type, e->getPosition().x, e->getPosition().y));
    }
    
    for (Entity* e : _dynamicEntities)
    {
        if (e->getEntityDef().bodyFlags & BodyFlag_Player)
        {
            // Don't save players into the map, since they are spawned dynamically by the server
            continue;
        }
        
        layout._entities.push_back(EntityInstanceDef(e->getID(), e->getEntityDef().type, e->getPosition().x, e->getPosition().y));
    }
    
    ENTITY_LAYOUT_MAPPER.saveEntityLayout(map, &layout);
    
    if (map != 'TEST')
    {
        // If the save was successful, update current map
        _map.key = map;
    }
}

void World::addEntity(Entity *e)
{
    if (isLayer(e))
    {
        _layers.push_back(e);
    }
    else if (isStatic(e))
    {
        e->initPhysics(getWorld());
        
        _staticEntities.push_back(e);
    }
    else if (isDynamic(e))
    {
        e->initPhysics(getWorld());
        
        _dynamicEntities.push_back(e);
        
        refreshPlayers();
    }
}

void World::removeEntity(Entity* e)
{
    if (isLayer(e))
    {
        removeEntity(e, _layers);
    }
    else if (isStatic(e))
    {
        e->deinitPhysics();
        
        removeEntity(e, _staticEntities);
    }
    else if (isDynamic(e))
    {
        e->deinitPhysics();
        
        removeEntity(e, _dynamicEntities);
        
        refreshPlayers();
    }
}

void World::stepPhysics()
{
    static int32 velocityIterations = 6;
    static int32 positionIterations = 2;
    
    // Instruct the world to perform a single step of simulation.
    // It is generally best to keep the time step and iterations fixed.
    _world->Step(FRAME_RATE, velocityIterations, positionIterations);
}

void World::clear()
{
    deinitPhysics(_staticEntities);
    
    GowUtil::cleanUpVectorOfPointers(_layers);
    GowUtil::cleanUpVectorOfPointers(_staticEntities);
    
    if (_flags & WorldFlag_Server || _flags & WorldFlag_Studio)
    {
        deinitPhysics(_dynamicEntities);
        GowUtil::cleanUpVectorOfPointers(_dynamicEntities);
        refreshPlayers();
    }
}

bool World::isMapLoaded()
{
    return _map.key > 0;
}

std::string& World::getMapName()
{
    return _map.name;
}

std::string& World::getMapFileName()
{
    return _map.fileName;
}

std::vector<Entity*>& World::getPlayers()
{
    return _players;
}

std::vector<Entity*>& World::getDynamicEntities()
{
    return _dynamicEntities;
}

std::vector<Entity*>& World::getStaticEntities()
{
    return _staticEntities;
}

std::vector<Entity*>& World::getLayers()
{
    return _layers;
}

b2World& World::getWorld()
{
    return *_world;
}

bool World::isLayer(Entity* e)
{
    return e->getEntityDef().fixtures.size() == 0 &&
    e->getEntityDef().bodyFlags == 0;
}

bool World::isStatic(Entity* e)
{
    return e->getEntityDef().fixtures.size() > 0 &&
    (e->getEntityDef().bodyFlags & BodyFlag_Static) &&
    !e->getEntityDef().stateSensitive;
}

bool World::isDynamic(Entity* e)
{
    return isDynamic(e->getEntityDef());
}

bool World::isDynamic(EntityDef& ed)
{
    return ed.fixtures.size() > 0 && (!(ed.bodyFlags & BodyFlag_Static) || ed.stateSensitive);
}

void World::refreshPlayers()
{
    _players.clear();
    
    for (Entity* e : _dynamicEntities)
    {
        if (e->getEntityDef().bodyFlags & BodyFlag_Player)
        {
            _players.push_back(e);
        }
    }
}

void World::removeEntity(Entity* e, std::vector<Entity*>& entities)
{
    assert(e);
    
    for (std::vector<Entity*>::iterator i = entities.begin(); i != entities.end(); )
    {
        if (e->getID() == (*i)->getID())
        {
            delete *i;
            
            i = entities.erase(i);
            return;
        }
        else
        {
            ++i;
        }
    }
}

void World::deinitPhysics(std::vector<Entity*>& entities)
{
    for (Entity* e : entities)
    {
        e->deinitPhysics();
    }
}

void EntityContactListener::BeginContact(b2Contact* contact)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    
    Entity* entityA = (Entity*)fixtureA->GetBody()->GetUserData().pointer;
    Entity* entityB = (Entity*)fixtureB->GetBody()->GetUserData().pointer;
    
    entityA->handleBeginContact(entityB, fixtureA, fixtureB);
    entityB->handleBeginContact(entityA, fixtureB, fixtureA);
}

void EntityContactListener::EndContact(b2Contact* contact)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    
    Entity* entityA = (Entity*)fixtureA->GetBody()->GetUserData().pointer;
    Entity* entityB = (Entity*)fixtureB->GetBody()->GetUserData().pointer;
    
    entityA->handleEndContact(entityB, fixtureA, fixtureB);
    entityB->handleEndContact(entityA, fixtureB, fixtureA);
}

bool EntityContactFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
    Entity* entityA = (Entity*)fixtureA->GetBody()->GetUserData().pointer;
    Entity* entityB = (Entity*)fixtureB->GetBody()->GetUserData().pointer;
    
    return entityA->shouldCollide(entityB, fixtureA, fixtureB) &&
    entityB->shouldCollide(entityA, fixtureB, fixtureA);
}
