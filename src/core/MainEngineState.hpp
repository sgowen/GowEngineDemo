//
//  MainEngineState.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "EngineState.hpp"

#include <string>

class MainRenderer;

enum MainEngineStateState
{
    MESS_Default =          1 << 0,
    MESS_InputName =        1 << 1,
    MESS_InputIpAddress =   1 << 2
};

class MainEngineState : public EngineState
{
    friend class MainRenderer;
    
public:
    static void create();
    static MainEngineState* getInstance();
    static void destroy();
    
    virtual void enter(Engine* e);
    virtual void execute(Engine* e);
    virtual void exit(Engine* e);
    
    virtual void createDeviceDependentResources();
    virtual void onWindowSizeChanged(int screenWidth, int screenHeight, int cursorWidth, int cursorHeight);
    virtual void releaseDeviceDependentResources();
    virtual void onResume();
    virtual void onPause();
    virtual void update();
    virtual void render();
    
private:
    static MainEngineState* s_instance;
    
    MainRenderer* _renderer;
    std::string _serverIPAddress;
    std::string _name;
    uint32_t _state;
    
    MainEngineState();
    ~MainEngineState();
    MainEngineState(const MainEngineState&);
    MainEngineState& operator=(const MainEngineState&);
};
