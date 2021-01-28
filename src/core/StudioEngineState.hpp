//
//  StudioEngineState.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/4/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "EngineState.hpp"

#include <stdint.h>

typedef void (*TestFunc)(Engine* e, uint32_t& testMap);

#include "StudioRenderer.hpp"
#include "World.hpp"

enum StudioEngineStateState
{
    SESS_DisplayBox2D =           1 <<  0,
    SESS_DisplayGrid =            1 <<  1,
    SESS_DisplayTypes =           1 <<  2,
    SESS_DisplayParallax =        1 <<  3,
    SESS_DisplayControls =        1 <<  4,
    SESS_DisplayAssets =          1 <<  5,
    SESS_DisplayEntities =        1 <<  6,
    SESS_DisplayNewMapDialog =    1 <<  7,
    SESS_DisplayLoadMapDialog =   1 << 10,
    SESS_DisplaySaveMapDialog =   1 << 11,
    SESS_DisplaySaveMapAsDialog = 1 << 12,
    SESS_Interpolation =          1 << 13,
    
    SESS_LayerBitBegin =               14,
    SESS_Layer0 =                 1 << 14,
    SESS_Layer1 =                 1 << 15,
    SESS_Layer2 =                 1 << 16,
    SESS_Layer3 =                 1 << 17,
    SESS_Layer4 =                 1 << 18,
    SESS_Layer5 =                 1 << 19,
    SESS_Layer6 =                 1 << 20,
    SESS_Layer7 =                 1 << 21,
    SESS_Layer8 =                 1 << 22,
    SESS_Layer9 =                 1 << 23,
    SESS_NumLayers =                   10,
    
    SESS_LayerAll = SESS_Layer0 | SESS_Layer1 | SESS_Layer2 | SESS_Layer3 | SESS_Layer4 | SESS_Layer5 | SESS_Layer6 | SESS_Layer7 | SESS_Layer8 | SESS_Layer9
};

enum StudioEngineStateTextInputType
{
    SETIT_Text =    1,
    SETIT_Integer = 2,
};

class StudioEngineState : public EngineState
{
    friend class StudioInputManager;
    friend class StudioRenderer;
    
public:
    static void create(TestFunc testFunc);
    static StudioEngineState* getInstance();
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
    virtual void render(double alpha);
    
private:
    static StudioEngineState* s_instance;
    
    StudioRenderer _renderer;
    World _world;
    Engine* _engine;
    TestFunc _testFunc;
    uint32_t _state;
    uint32_t _textInputType;
    
    StudioEngineState(TestFunc testFunc);
    ~StudioEngineState();
    StudioEngineState(const StudioEngineState&);
    StudioEngineState& operator=(const StudioEngineState&);
};
