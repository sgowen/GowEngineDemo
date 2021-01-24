//
//  MainEngineState.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "EngineState.hpp"

class Renderer;

class MainEngineState : public EngineState
{
public:
    static void create();
    static MainEngineState* getInstance();
    static void destroy();
    
    virtual void enter(Engine* engine);
    virtual void execute(Engine* engine);
    virtual void exit(Engine* engine);
    
    virtual void createDeviceDependentResources();
    virtual void onWindowSizeChanged(int screenWidth, int screenHeight, int cursorWidth, int cursorHeight);
    virtual void releaseDeviceDependentResources();
    virtual void onResume();
    virtual void onPause();
    virtual void update();
    virtual void render();
    
private:
    static MainEngineState* s_instance;
    
    Renderer* _renderer;
    
    MainEngineState();
    ~MainEngineState();
    MainEngineState(const MainEngineState&);
    MainEngineState& operator=(const MainEngineState&);
};
