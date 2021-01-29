//
//  GameEngineState.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "GameEngineState.hpp"

#include "Entity.hpp"
#include "GameRenderer.hpp"
#include "World.hpp"
#include "Timing.hpp"
#include "GameInputManager.hpp"
#include "Server.hpp"

#include "Constants.hpp"
#include "CursorConverter.hpp"
#include "StringUtil.hpp"
#include "MathUtil.hpp"
#include "NetworkManagerClient.hpp"
#include "NetworkManagerServer.hpp"
#include "GameInputState.hpp"
#include "EntityManager.hpp"
#include "SocketClientHelper.hpp"
#include "GowAudioEngine.hpp"
#include "Assets.hpp"
#include "Constants.hpp"
#include "FPSUtil.hpp"
#include "CursorConverter.hpp"
#include "EntityMapper.hpp"
#include "EntityLayoutMapper.hpp"
#include "MainConfig.hpp"
#include "Util.hpp"
#include "PlayerController.hpp"
#include "InstanceManager.hpp"
#include "Config.hpp"
#include "Macros.hpp"
#include "Engine.hpp"

GameEngineState* GameEngineState::s_instance = NULL;

void GameEngineState::create()
{
    assert(!s_instance);
    
    s_instance = new GameEngineState();
}

GameEngineState* GameEngineState::getInstance()
{
    return s_instance;
}

void GameEngineState::destroy()
{
    assert(s_instance);
    
    delete s_instance;
    s_instance = NULL;
}

uint64_t GameEngineState::sGetPlayerAddressHash(uint8_t inPlayerIndex)
{
    uint64_t ret = 0;
    
    GameEngineState* engine = GameEngineState::getInstance();
    assert(engine);
    
    World* world = engine->_world;
    assert(world);
    
    uint8_t playerID = inPlayerIndex + 1;
    
    Entity* player = NULL;
    for (Entity* e : world->getPlayers())
    {
        PlayerController* robot = static_cast<PlayerController*>(e->getController());
        if (robot->getPlayerID() == playerID)
        {
            player = e;
            break;
        }
    }
    
    if (player)
    {
        PlayerController* robot = static_cast<PlayerController*>(player->getController());
        assert(robot);
        
        ret = robot->getAddressHash();
    }
    
    return ret;
}

void GameEngineState::sHandleDynamicEntityCreatedOnClient(Entity* entity)
{
    GameEngineState* engine = GameEngineState::getInstance();
    assert(engine);
    
    World* world = engine->_world;
    assert(world);
    
    world->addEntity(entity);
}

void GameEngineState::sHandleDynamicEntityDeletedOnClient(Entity* entity)
{
    GameEngineState* engine = GameEngineState::getInstance();
    assert(engine);
    
    World* world = engine->_world;
    assert(world);
    
    world->removeEntity(entity);
}

void GameEngineState::sHandleTest(Engine* e, uint32_t& testMap)
{
    Server::create(ServerFlag_TestSession, &testMap);
    
    GameEngineState* ge = GameEngineState::getInstance();
    ge->_serverIPAddress = std::string("localhost:9999");
    ge->_name = std::string("TESTER");
    ge->_isHost = true;
    SET_BIT(ge->_state, GESS_Host, true);
    SET_BIT(ge->_state, GESS_Connected, false);
    
    e->getStateMachine().changeState(ge);
}

void GameEngineState::sHandleHostServer(Engine* e, std::string inName)
{
    Server::create();
    
    GameEngineState* ge = GameEngineState::getInstance();
    ge->_name = inName;
    ge->_isHost = true;
    SET_BIT(ge->_state, GESS_Host, true);
    SET_BIT(ge->_state, GESS_Connected, false);
    
    e->getStateMachine().changeState(ge);
}

void GameEngineState::sHandleJoinServer(Engine* e, std::string inServerIPAddress, std::string inName)
{
    GameEngineState* ge = GameEngineState::getInstance();
    ge->_serverIPAddress = inServerIPAddress;
    ge->_name = inName;
    ge->_isHost = false;
    ge->joinServer();
    
    e->getStateMachine().changeState(ge);
}

void GameEngineState::enter(Engine* e)
{
    // TODO, this is ugly
    _engine = e;
    
    createDeviceDependentResources();
    onWindowSizeChanged(e->screenWidth(), e->screenHeight(), e->cursorWidth(), e->cursorHeight());
    
    _map = 0;
    _world = new World(WorldFlag_Client);
    _timing->reset();
    _input = GameInputManager::getInstance();
    _server = Server::getInstance();
}

void GameEngineState::execute(Engine* e)
{
    // Empty
}

void GameEngineState::exit(Engine* e)
{
    releaseDeviceDependentResources();
    
    _input = NULL;
    
    delete _world;
    
    if (NW_MGR_CLIENT)
    {
        NetworkManagerClient::destroy();
    }
    
    if (_server)
    {
        Server::destroy();
    }
    
    _timing->reset();
    
    SET_BIT(_state, GESS_Host, false);
    SET_BIT(_state, GESS_Connected, false);
}

void GameEngineState::createDeviceDependentResources()
{
    GameInputManager::create();
    
    ENTITY_MAPPER.initWithJSONFile("json/entities.json");
    ENTITY_LAYOUT_MAPPER.initWithJSONFile("json/maps.json");
    ASSETS.initWithJSONFile("json/assets_game.json");
    
    CURSOR_CONVERTER.setMatrixSize(CFG_MAIN._camWidth, CFG_MAIN._camHeight);
    
    _renderer.createDeviceDependentResources();
    
    GowAudioEngine::create();
    GOW_AUDIO->loadFromAssets();
    GOW_AUDIO->playMusic();
}

void GameEngineState::onWindowSizeChanged(int screenWidth, int screenHeight, int cursorWidth, int cursorHeight)
{
    _renderer.onWindowSizeChanged(screenWidth, screenHeight);
    
    CURSOR_CONVERTER.setCursorSize(cursorWidth, cursorHeight);
}

void GameEngineState::releaseDeviceDependentResources()
{
    GowAudioEngine::destroy();
    
    _renderer.releaseDeviceDependentResources();
    
    GameInputManager::destroy();
}

void GameEngineState::onResume()
{
    GOW_AUDIO->resume();
}

void GameEngineState::onPause()
{
    GOW_AUDIO->pause();
}

void GameEngineState::update()
{
    if (_state & GESS_Host &&
        !(_state & GESS_Connected))
    {
        if (NW_MGR_SERVER && NW_MGR_SERVER->isConnected())
        {
            _serverIPAddress = std::string("localhost:9999");
            
            joinServer();
        }
        
        return;
    }
    
    _timing->onFrame();
    
    NW_MGR_CLIENT->processIncomingPackets();
    if (NW_MGR_CLIENT->state() == NCS_Disconnected)
    {
        _engine->getStateMachine().revertToPreviousState();
        return;
    }
    else if (NW_MGR_CLIENT->hasReceivedNewState())
    {
        uint32_t map = NW_MGR_CLIENT->getMap();
        if (_map != map)
        {
            _map = map;
            _world->loadMap(_map);
        }
        
        for (Entity* e : _world->getDynamicEntities())
        {
            e->getNetworkController()->recallNetworkCache();
        }
        
        // all processed moves have been removed, so all that are left are unprocessed moves so we must apply them...
        for (const Move& move : _input->getMoveList())
        {
            updateWorld(&move);
        }
    }
    
    _input->update();
    if (_input->getMenuState() == GIMS_ESCAPE)
    {
        _engine->getStateMachine().revertToPreviousState();
        return;
    }
    
    _isLive = true;
    updateWorld(_input->getPendingMove());
    _isLive = false;
    
    _input->clearPendingMove();
    NW_MGR_CLIENT->sendOutgoingPackets();
    
    if (_isHost)
    {
        _server->update();
    }
}

void GameEngineState::render(double alpha)
{
    if (_state & GESS_Interpolation)
    {
        for (Entity* entity : _world->getPlayers())
        {
            entity->interpolate(alpha);
        }
    }
    
    _renderer.render();
    
    if (_state & GESS_Interpolation)
    {
        for (Entity* entity : _world->getPlayers())
        {
            entity->endInterpolation();
        }
    }
    
    GOW_AUDIO->render();
}

World* GameEngineState::getWorld()
{
    return _world;
}

bool GameEngineState::isLive()
{
    return _isLive;
}

void GameEngineState::joinServer()
{
    NetworkManagerClient::create(new SocketClientHelper(_serverIPAddress, _name, CFG_MAIN._clientPort, NW_MGR_CLIENT_CALLBACKS), GAME_ENGINE_CALLBACKS, INPUT_MANAGER_CALLBACKS);
    
    assert(NW_MGR_CLIENT);
    
    SET_BIT(_state, GESS_Connected, true);
}

void GameEngineState::updateWorld(const Move* move)
{
    assert(move);
    
    for (Entity* e : _world->getPlayers())
    {
        PlayerController* robot = static_cast<PlayerController*>(e->getController());
        robot->processInput(move->getInputState());
    }
    
    _world->stepPhysics();
    
    for (Entity* e : _world->getDynamicEntities())
    {
        e->update();
    }
}

GameEngineState::GameEngineState() : EngineState(),
_renderer(),
_world(NULL),
_engine(NULL),
_timing(static_cast<Timing*>(INSTANCE_MGR.get(INSTANCE_TIME_CLIENT))),
_input(NULL),
_state(GESS_Default),
_map(0),
_isHost(false),
_isLive(false)
{
    _state |= GESS_Interpolation | GESS_Lighting | GESS_DisplayUI;
}

GameEngineState::~GameEngineState()
{
    // Empty
}
