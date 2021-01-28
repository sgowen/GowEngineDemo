//
//  GameEngineState.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include <stdint.h>
#include <string>

#include "EngineState.hpp"
#include "GameRenderer.hpp"

#define GAME_ENGINE_CALLBACKS GameEngineState::sHandleDynamicEntityCreatedOnClient, GameEngineState::sHandleDynamicEntityDeletedOnClient

class Entity;
class World;
class Timing;
class GameInputManager;
class Move;
class Server;

enum GameEngineStateState
{
    GESS_Default =       0,
    GESS_Host =          1 << 0,
    GESS_Connected =     1 << 1,
    GESS_DisplayBox2D =  1 << 2,
    GESS_Interpolation = 1 << 3,
    GESS_Lighting =      1 << 4,
    GESS_DisplayUI =     1 << 5
};

class GameEngineState : public EngineState
{
    friend class GameInputManager;
    friend class GameRenderer;
    
public:
    static void create();
    static GameEngineState* getInstance();
    static void destroy();
    
    static uint64_t sGetPlayerAddressHash(uint8_t inPlayerIndex);
    static void sHandleDynamicEntityCreatedOnClient(Entity* entity);
    static void sHandleDynamicEntityDeletedOnClient(Entity* entity);
    static void sHandleTest(Engine* e, uint32_t& testMap);
    static void sHandleHostServer(Engine* e, std::string inName);
    static void sHandleJoinServer(Engine* e, std::string inServerIPAddress, std::string inName);
    
    virtual void enter(Engine* e);
    virtual void execute(Engine* e);
    virtual void exit(Engine* e);
    
    virtual void createDeviceDependentResources();
    virtual void onWindowSizeChanged(int screenWidth, int screenHeight, int cursorWidth, int cursorHeight);
    virtual void releaseDeviceDependentResources();
    virtual void onResume();
    virtual void onPause();
    virtual void update();
    virtual void render(double alpha);
    
    void test(uint32_t map);
    World* getWorld();
    bool isLive();
    
private:
    static GameEngineState* s_instance;
    
    GameRenderer _renderer;
    World* _world;
    Engine* _engine;
    Timing* _timing;
    Server* _server;
    GameInputManager* _input;
    uint32_t _state;
    std::string _serverIPAddress;
    std::string _name;
    uint32_t _map;
    bool _isHost;
    bool _isLive;
    
    void joinServer();
    void updateWorld(const Move* move);
    
    GameEngineState();
    ~GameEngineState();
    GameEngineState(const GameEngineState&);
    GameEngineState& operator=(const GameEngineState&);
};
