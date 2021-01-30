//
//  Server.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 5/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "Server.hpp"

#include "World.hpp"
#include "ClientProxy.hpp"
#include "Entity.hpp"
#include "Timing.hpp"
#include "EntityIDManager.hpp"

#include "NetworkManagerServer.hpp"
#include "EntityManager.hpp"
#include "SocketServerHelper.hpp"
#include "MachineAddress.hpp"
#include "Constants.hpp"
#include "PlayerController.hpp"
#include "EntityMapper.hpp"
#include "EntityLayoutMapper.hpp"
#include "InstanceManager.hpp"
#include "MainConfig.hpp"

#include <box2d/box2d.h>

#include <ctime>
#include <assert.h>

#define SERVER_CALLBACKS Server::sHandleNewClient, Server::sHandleLostClient, Server::sHandleInputStateCreation, Server::sHandleInputStateRelease

Server* Server::s_instance = NULL;

void Server::create(uint32_t flags, void* data)
{
    assert(!s_instance);
    
    s_instance = new Server(flags, data);
}

Server * Server::getInstance()
{
    return s_instance;
}

void Server::destroy()
{
    assert(s_instance);
    
    delete s_instance;
    s_instance = NULL;
}

void Server::sHandleNewClient(uint8_t playerID, std::string playerName)
{
    getInstance()->handleNewClient(playerID, playerName);
}

void Server::sHandleLostClient(ClientProxy* inClientProxy, uint8_t index)
{
    getInstance()->handleLostClient(inClientProxy, index);
}

InputState* Server::sHandleInputStateCreation()
{
    return getInstance()->handleInputStateCreation();
}

void Server::sHandleInputStateRelease(InputState* inputState)
{
    getInstance()->handleInputStateRelease(inputState);
}

void Server::update()
{
    _timing->onFrame();
    
    NW_MGR_SERVER->processIncomingPackets();
    
    int moveCount = NW_MGR_SERVER->getMoveCount();
    if (moveCount > 0)
    {
        for (int i = 0; i < moveCount; ++i)
        {
            for (Entity* entity : _world->getPlayers())
            {
                PlayerController* robot = static_cast<PlayerController*>(entity->getController());
                
                ClientProxy* client = NW_MGR_SERVER->getClientProxy(robot->getPlayerID());
                if (client)
                {
                    MoveList& moveList = client->getUnprocessedMoveList();
                    Move* move = moveList.getMoveAtIndex(i);
                    if (move)
                    {
                        robot->processInput(move->getInputState());
                        
                        moveList.markMoveAsProcessed(move);
                        
                        client->setLastMoveTimestampDirty(true);
                    }
                }
            }
            
            _world->stepPhysics();
            
            std::vector<Entity*> toDelete;
            
            for (Entity* e : _world->getDynamicEntities())
            {
                if (!e->isRequestingDeletion())
                {
                    e->update();
                }
                
                if (e->isRequestingDeletion())
                {
                    toDelete.push_back(e);
                }
            }
            
            for (Entity* e : toDelete)
            {
                NW_MGR_SERVER->deregisterEntity(e);
                
                EntityController* c = e->getController();
                if (c->getRTTI().derivesFrom(PlayerController::rtti))
                {
                    PlayerController* robot = static_cast<PlayerController*>(c);
                    assert(robot);
                    
                    // This is my shoddy respawn implementation
                    getInstance()->spawnRobotForPlayer(robot->getPlayerID(), robot->getPlayerName());
                }
                
                _world->removeEntity(e);
            }
            
            handleDirtyStates(_world->getDynamicEntities());
        }
        
        for (uint8_t i = 0; i < MAX_NUM_PLAYERS_PER_SERVER; ++i)
        {
            uint8_t playerID = i + 1;
            ClientProxy* client = NW_MGR_SERVER->getClientProxy(playerID);
            if (client)
            {
                MoveList& moveList = client->getUnprocessedMoveList();
                moveList.removeProcessedMoves(client->getUnprocessedMoveList().getLastProcessedMoveTimestamp(), Server::sHandleInputStateRelease);
            }
        }
    }
    
    NW_MGR_SERVER->sendOutgoingPackets();
}

void Server::toggleMap()
{
    if (_flags & ServerFlag_TestSession)
    {
        return;
    }
    
    _map = _map == 'Z001' ? 'Z002' : 'Z001';
    
    loadMap();
}

uint32_t Server::getFlags()
{
    return _flags;
}

World* Server::getWorld()
{
    return _world;
}

void Server::handleNewClient(uint8_t playerID, std::string playerName)
{
    if (NW_MGR_SERVER->getNumClientsConnected() == 1)
    {
        // This is our first client!
        
        if (_flags & ServerFlag_TestSession)
        {
            uint32_t* map = static_cast<uint32_t*>(_data);
            _map = *map;
            loadMap();
        }
        else
        {
            if (_map == 0)
            {
                _map = 'Z002';
                
                loadMap();
            }
        }
    }
    
    spawnRobotForPlayer(playerID, playerName);
}

void Server::handleLostClient(ClientProxy* inClientProxy, uint8_t index)
{
    if (index >= 1)
    {
        uint8_t playerID = inClientProxy->getPlayerID(index);
        
        deleteRobotWithPlayerId(playerID);
    }
    else
    {
        for (int i = 0; i < inClientProxy->getNumPlayers(); ++i)
        {
            uint8_t playerID = inClientProxy->getPlayerID(i);
            
            deleteRobotWithPlayerId(playerID);
        }
    }
}

InputState* Server::handleInputStateCreation()
{
    InputState* ret = _inputStates.obtain();
    ret->reset();
    
    return ret;
}

void Server::handleInputStateRelease(InputState* inputState)
{
    GameInputState* mainInputState = static_cast<GameInputState*>(inputState);
    _inputStates.free(mainInputState);
}

void Server::deleteRobotWithPlayerId(uint8_t playerID)
{
    for (Entity* e : _world->getPlayers())
    {
        PlayerController* robot = static_cast<PlayerController*>(e->getController());
        if (robot->getPlayerID() == playerID)
        {
            e->requestDeletion();
            break;
        }
    }
}

void Server::spawnRobotForPlayer(uint8_t inPlayerId, std::string inPlayerName)
{
    ClientProxy* client = NW_MGR_SERVER->getClientProxy(inPlayerId);
    if (!client)
    {
        return;
    }
    
    float spawnX = 32;
    float spawnY = 16;
    for (Entity* e : _world->getLayers())
    {
        if (e->getEntityDef()._key == 'Z1S1')
        {
            spawnX = e->getPosition().x;
            spawnY = e->getPosition().y - e->getHeight() / 2 + 9;
            break;
        }
    }
    
    EntityInstanceDef eid(_entityIDManager->getNextDynamicEntityID(), 'ROBT', spawnX, spawnY);
    Entity* e = ENTITY_MAPPER.createEntity(&eid, true);
    PlayerController* robot = static_cast<PlayerController*>(e->getController());
    robot->setAddressHash(client->getMachineAddress()->getHash());
    robot->setPlayerName(inPlayerName);
    robot->setPlayerID(inPlayerId);
    
    _world->addEntity(e);
    
    NW_MGR_SERVER->registerEntity(e);
}

void Server::loadMap()
{
    std::vector<uint8_t> playerIDs;
    std::vector<std::string> playerNames;
    for (Entity* e : _world->getPlayers())
    {
        PlayerController* robot = static_cast<PlayerController*>(e->getController());
        playerIDs.push_back(robot->getPlayerID());
        playerNames.push_back(robot->getPlayerName());
    }
    
    for (Entity* e : _world->getDynamicEntities())
    {
        NW_MGR_SERVER->deregisterEntity(e);
    }
    
    _world->loadMap(_map);
    
    for (Entity* e : _world->getDynamicEntities())
    {
        NW_MGR_SERVER->registerEntity(e);
    }
    
    NW_MGR_SERVER->setMap(_map);
    
    for (int i = 0; i < playerIDs.size() && i < playerNames.size(); ++i)
    {
        handleNewClient(playerIDs[i], playerNames[i]);
    }
}

void Server::handleDirtyStates(std::vector<Entity*>& entities)
{
    for (Entity* e : entities)
    {
        uint16_t dirtyState = e->getNetworkController()->getDirtyState();
        if (dirtyState > 0)
        {
            NW_MGR_SERVER->setStateDirty(e->getID(), dirtyState);
        }
    }
}

Server::Server(uint32_t flags, void* data) :
_flags(flags),
_data(data),
_timing(static_cast<Timing*>(INSTANCE_MGR.get(INSTANCE_TIME_SERVER))),
_entityIDManager(static_cast<EntityIDManager*>(INSTANCE_MGR.get(INSTANCE_ENTITY_ID_MANAGER_SERVER))),
_world(new World(WorldFlag_Server)),
_map(0)
{
    _timing->reset();
    
    NetworkManagerServer::create(new SocketServerHelper(CFG_MAIN._serverPort, NW_MGR_SERVER_CALLBACKS), SERVER_CALLBACKS);
    
    assert(NW_MGR_SERVER != NULL);
}

Server::~Server()
{
    delete _world;
    NetworkManagerServer::destroy();
}
