//
//  MainEngineState.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "MainEngineState.hpp"

#include "MainRenderer.hpp"

#include "Assets.hpp"
#include "Macros.hpp"
#include "MainInputManager.hpp"
#include "MainConfig.hpp"
#include "Engine.hpp"
#include "StringUtil.hpp"
#include "StudioEngineState.hpp"
#include "GameEngineState.hpp"

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

void MainEngineState::enter(Engine* e)
{
    createDeviceDependentResources();
    onWindowSizeChanged(e->screenWidth(), e->screenHeight(), e->cursorWidth(), e->cursorHeight());
    
    _state = MESS_Default;
}

void MainEngineState::execute(Engine* e)
{
    // TODO, this is ugly
    _engine = e;
}

void MainEngineState::exit(Engine* e)
{
    releaseDeviceDependentResources();
}

void MainEngineState::createDeviceDependentResources()
{
    MainInputManager::create();
    
    CFG_MAIN.init();
    ASSETS.initWithJSONFile("json/assets_main.json");
    
    _renderer->createDeviceDependentResources();
}

void MainEngineState::onWindowSizeChanged(int screenWidth, int screenHeight, int cursorWidth, int cursorHeight)
{
    _renderer->onWindowSizeChanged(screenWidth, screenHeight);
}

void MainEngineState::releaseDeviceDependentResources()
{
    MainInputManager::destroy();
    
    _renderer->releaseDeviceDependentResources();
}

void MainEngineState::onResume()
{
    // Empty
}

void MainEngineState::onPause()
{
    // Empty
}

void MainEngineState::update()
{
    MainInputManager* mim = MainInputManager::getInstance();
    
    mim->update();
    
    int menuState = mim->getMenuState();
    
    if (mim->isLiveMode())
    {
        if (menuState == MIMS_ESCAPE)
        {
            mim->setLiveInputMode(false);
            
            _state = MESS_Default;
        }
        else if (mim->isTimeToProcessInput())
        {
            bool needsToProcessInput = true;
            if (_state == MESS_InputIpAddress)
            {
                _serverIPAddress = StringUtil::format("%s:%d", mim->getLiveInput().c_str(), CFG_MAIN._serverPort);
                _name.clear();
                _state = MESS_InputName;
            }
            else if (_state == MESS_InputName)
            {
                _name = mim->getLiveInput();
                if (_name.length() > 0)
                {
                    mim->setLiveInputMode(false);
                    
                    if (_serverIPAddress.length() == 0)
                    {
                        GameEngineState::sHandleHostServer(_engine, _name);
                    }
                    else
                    {
                        GameEngineState::sHandleJoinServer(_engine, _serverIPAddress, _name);
                    }
                    
                    needsToProcessInput = false;
                }
            }
            
            if (needsToProcessInput)
            {
                mim->onInputProcessed();
            }
        }
    }
    else
    {
        if (menuState == MIMS_ENTER_STUDIO)
        {
            _engine->getStateMachine().changeState(StudioEngineState::getInstance());
        }
        else if (menuState == MIMS_START_SERVER)
        {
            _serverIPAddress.clear();
            _name.clear();
            _state = MESS_InputName;
            mim->setLiveInputMode(true);
        }
        else if (menuState == MIMS_JOIN_LOCAL_SERVER)
        {
            _serverIPAddress.clear();
            _state = MESS_InputIpAddress;
            mim->setLiveInputMode(true);
        }
        else if (menuState == MIMS_ESCAPE)
        {
            _engine->setRequestedAction(REQUESTED_ACTION_EXIT);
        }
    }
}

void MainEngineState::render(double alpha)
{
    UNUSED(alpha);
    
    _renderer->render(*this);
}

MainEngineState* MainEngineState::s_instance = NULL;

MainEngineState::MainEngineState() : EngineState(),
_engine(NULL),
_renderer(new MainRenderer()),
_serverIPAddress(""),
_name(""),
_state(MESS_Default)
{
    // Empty
}

MainEngineState::~MainEngineState()
{
    delete _renderer;
}
