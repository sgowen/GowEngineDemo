//
//  MainInputManager.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 5/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include <string>

enum MainInputManagerState
{
    MIMS_NONE = 0,
    MIMS_ENTER_STUDIO,
    MIMS_START_SERVER,
    MIMS_JOIN_LOCAL_SERVER,
    MIMS_ESCAPE
};

class MainInputManager
{
public:
    static void create();
    static MainInputManager* getInstance();
    static void destroy();
    
    void update();
    void setLiveInputMode(bool isLiveMode);
    bool isLiveMode();
    bool isTimeToProcessInput();
    void onInputProcessed();
    int getMenuState();
    std::string& getLiveInputRef();
    std::string getLiveInput();
    
private:
    static MainInputManager* s_instance;
    
    std::string _liveInput;
    int _inputState;
    bool _isLiveMode;
    bool _isTimeToProcessInput;
    
    bool isTimeToSampleInput();
    
    MainInputManager();
    ~MainInputManager();
    MainInputManager(const MainInputManager&);
    MainInputManager& operator=(const MainInputManager&);
};
