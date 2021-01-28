//
//  StudioEngineState.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/4/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "StudioEngineState.hpp"

#include "Constants.hpp"
#include "CursorConverter.hpp"
#include "StringUtil.hpp"
#include "MathUtil.hpp"
#include "StudioInputManager.hpp"
#include "EntityManager.hpp"
#include "Assets.hpp"
#include "Constants.hpp"
#include "FPSUtil.hpp"
#include "CursorConverter.hpp"
#include "EntityMapper.hpp"
#include "EntityLayoutMapper.hpp"
#include "Macros.hpp"
#include "Config.hpp"
#include "MainConfig.hpp"
#include "Engine.hpp"

StudioEngineState* StudioEngineState::s_instance = NULL;

void StudioEngineState::create(TestFunc testFunc)
{
    assert(!s_instance);
    
    s_instance = new StudioEngineState(testFunc);
}

StudioEngineState * StudioEngineState::getInstance()
{
    return s_instance;
}

void StudioEngineState::destroy()
{
    assert(s_instance);
    
    delete s_instance;
    s_instance = NULL;
}

void StudioEngineState::enter(Engine* e)
{
    // TODO, this is ugly
    _engine = e;
    
    createDeviceDependentResources();
    onWindowSizeChanged(e->screenWidth(), e->screenHeight(), e->cursorWidth(), e->cursorHeight());
}

void StudioEngineState::execute(Engine* e)
{
    // Empty
}

void StudioEngineState::exit(Engine* e)
{
    releaseDeviceDependentResources();
}

void StudioEngineState::createDeviceDependentResources()
{
    StudioInputManager::create();
    
    ENTITY_MAPPER.initWithJSONFile("json/entities.json");
    ENTITY_LAYOUT_MAPPER.initWithJSONFile("json/maps.json");
    ASSETS.initWithJSONFile("json/assets_game.json");
    
    CURSOR_CONVERTER.setMatrixSize(CFG_MAIN._camWidth, CFG_MAIN._camHeight);
    
    _renderer.createDeviceDependentResources();
}

void StudioEngineState::onWindowSizeChanged(int screenWidth, int screenHeight, int cursorWidth, int cursorHeight)
{
    _renderer.onWindowSizeChanged(screenWidth, screenHeight);
    
    CURSOR_CONVERTER.setCursorSize(cursorWidth, cursorHeight);
}

void StudioEngineState::releaseDeviceDependentResources()
{
    _renderer.releaseDeviceDependentResources();
    
    StudioInputManager::destroy();
}

void StudioEngineState::onResume()
{
    // Empty
}

void StudioEngineState::onPause()
{
    // Empty
}

void StudioEngineState::update()
{
    StudioInputManager* sim = StudioInputManager::getInstance();
    
    sim->update();
    if (sim->getMenuState() == SIMS_ESCAPE)
    {
        _engine->getStateMachine().revertToPreviousState();
    }
}

void StudioEngineState::render(double alpha)
{
    UNUSED(alpha);
    
    _renderer.render();
}

StudioEngineState::StudioEngineState(TestFunc testFunc) : EngineState(),
_renderer(),
_world(WorldFlag_Studio),
_engine(NULL),
_testFunc(testFunc),
_state(SESS_DisplayGrid | SESS_DisplayControls | SESS_LayerAll),
_textInputType(0)
{
    // Empty
}

StudioEngineState::~StudioEngineState()
{
    // Empty
}
