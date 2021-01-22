//
//  MainEngineState.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "MainEngineState.hpp"

#include "Assets.hpp"
#include "Macros.hpp"
#include "GowAudioEngine.hpp"
#include "InputManager.hpp"

#include <stdlib.h>
#include <assert.h>

void MainEngineState::create()
{
    assert(s_instance == NULL);
    
    s_instance = new MainEngineState();
}

MainEngineState* MainEngineState::getInstance()
{
    return s_instance;
}

void MainEngineState::destroy()
{
    assert(s_instance != NULL);
    
    delete s_instance;
    s_instance = NULL;
}

void MainEngineState::enter(Engine* engine)
{
    // TODO
}

void MainEngineState::execute(Engine* engine)
{
    // TODO
}

void MainEngineState::exit(Engine* engine)
{
    // TODO
}

void MainEngineState::createDeviceDependentResources()
{
    ASSETS.initWithJSONFile("json/assets.json");
    
    _renderer.createDeviceDependentResources();
    
    GowAudioEngine::create();
    GOW_AUDIO->loadFromAssets();
    GOW_AUDIO->playMusic();
}

void MainEngineState::onWindowSizeChanged(int screenWidth, int screenHeight, int cursorWidth, int cursorHeight)
{
    _renderer.onWindowSizeChanged(screenWidth, screenHeight);
}

void MainEngineState::releaseDeviceDependentResources()
{
    GowAudioEngine::destroy();
    
    _renderer.releaseDeviceDependentResources();
    
    ASSETS.clear();
}

void MainEngineState::onResume()
{
    GOW_AUDIO->resume();
}

void MainEngineState::onPause()
{
    GOW_AUDIO->pause();
}

void MainEngineState::update()
{
    GOW_AUDIO->update();
    
    std::vector<CursorEvent*>& cursorEvents = INPUT_MGR.getCursorEvents();
    for (auto e : cursorEvents)
    {
        if (e->_type == CursorEventType_UP)
        {
            GOW_AUDIO->playSound(1);
        }
    }
}

void MainEngineState::render()
{
    _renderer.render();
    GOW_AUDIO->render();
}

MainEngineState* MainEngineState::s_instance = NULL;

MainEngineState::MainEngineState() : EngineState(),
_renderer()
{
    // TODO
}

MainEngineState::~MainEngineState()
{
    // TODO
}
