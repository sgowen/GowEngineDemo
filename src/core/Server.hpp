//
//  Server.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 5/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "Pool.hpp"
#include "GameInputState.hpp"

#include <string>
#include <vector>

enum ServerFlag
{
    ServerFlag_Default =     0,
    ServerFlag_TestSession = 1 << 0
};

class World;
class ClientProxy;
class Entity;
class Timing;
class EntityIDManager;

class Server
{
public:
    static void create(uint32_t flags = ServerFlag_Default, void* data = NULL);
    static Server* getInstance();
    static void destroy();
    
    static void sHandleDynamicEntityCreatedOnServer(Entity* entity);
    static void sHandleDynamicEntityDeletedOnServer(Entity* entity);
    static void sHandleNewClient(uint8_t playerID, std::string playerName);
    static void sHandleLostClient(ClientProxy* inClientProxy, uint8_t index);
    static InputState* sHandleInputStateCreation();
    static void sHandleInputStateRelease(InputState* inputState);
    
    void update();
    uint8_t getPlayerIDForRobotBeingCreated();
    void toggleMap();
    uint32_t getFlags();
    World* getWorld();

private:
    static Server* s_instance;
    
    uint32_t _flags;
    void* _data;
    Timing* _timing;
    EntityIDManager* _entityIDManager;
    World* _world;
    Pool<GameInputState> _inputStates;
    uint32_t _map;
    
    void handleNewClient(uint8_t playerID, std::string playerName);
    void handleLostClient(ClientProxy* inClientProxy, uint8_t index);
    InputState* handleInputStateCreation();
    void handleInputStateRelease(InputState* inputState);
    void deleteRobotWithPlayerId(uint8_t playerID);
    void spawnRobotForPlayer(uint8_t inPlayerId, std::string inPlayerName);
    void loadMap();
    void handleDirtyStates(std::vector<Entity*>& entities);
    
    Server(uint32_t flags = ServerFlag_Default, void* data = NULL);
    ~Server();
    Server(const Server&);
    Server& operator=(const Server&);
};
