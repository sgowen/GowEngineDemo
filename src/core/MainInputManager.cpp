//
//  MainInputManager.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 5/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "MainInputManager.hpp"

#include "InputManager.hpp"
#include "KeyboardLookup.hpp"
#include "StringUtil.hpp"
#include "Constants.hpp"

#include <sstream>

MainInputManager* MainInputManager::s_instance = NULL;

void MainInputManager::create()
{
    assert(!s_instance);
    
    s_instance = new MainInputManager();
}

MainInputManager * MainInputManager::getInstance()
{
    return s_instance;
}

void MainInputManager::destroy()
{
    assert(s_instance);
    
    delete s_instance;
    s_instance = NULL;
}

void MainInputManager::update()
{
    _inputState = MIMS_NONE;
    
    if (_isLiveMode)
    {
        std::stringstream ss;
        
        std::vector<KeyboardEvent*>& keyboardEvents = INPUT_MGR.getKeyboardEvents();
        for (std::vector<KeyboardEvent *>::iterator i = keyboardEvents.begin(); i != keyboardEvents.end(); ++i)
        {
            KeyboardEvent* keyboardEvent = (*i);
            if (isKeySupported(keyboardEvent->_key))
            {
                if (keyboardEvent->isDown()
                    && !keyboardEvent->isHeld())
                {
                    if (keyboardEvent->_key == GOW_KEY_CARRIAGE_RETURN)
                    {
                        _isTimeToProcessInput = true;
                        return;
                    }
                    else if (keyboardEvent->_key == GOW_KEY_ESCAPE)
                    {
                        return;
                    }
                    else if (keyboardEvent->_key == GOW_KEY_BACK_SPACE
                             || keyboardEvent->_key == GOW_KEY_DELETE)
                    {
                        std::string s = ss.str();
                        _liveInput += s;
                        if (_liveInput.end() > _liveInput.begin())
                        {
                            _liveInput.erase(_liveInput.end() - 1, _liveInput.end());
                        }
                        return;
                    }
                    else
                    {
                        char key = keyboardEvent->_key;
                        ss << StringUtil::format("%c", key);
                    }
                }
                else if (keyboardEvent->isUp())
                {
                    if (keyboardEvent->_key == GOW_KEY_ESCAPE)
                    {
                        _inputState = MIMS_ESCAPE;
                        return;
                    }
                }
            }
        }
        
#if IS_MOBILE
        std::vector<CursorEvent*>& cursorEvents = INPUT_MGR.getCursorEvents();
        for (std::vector<CursorEvent *>::iterator i = cursorEvents.begin(); i != cursorEvents.end(); ++i)
        {
            if ((*i)->_type == CursorEventType_UP)
            {
                if (_liveInput.length() > 0)
                {
                    _isTimeToProcessInput = true;
                }
                else
                {
                    ss << "mobile";
                }
            }
            else
            {
                continue;
            }
        }
#endif
        
        std::string s = ss.str();
        _liveInput += s;
        if (_liveInput.length() > MAX_USER_NAME_LENGTH)
        {
            int sub = static_cast<int>(_liveInput.length()) - MAX_USER_NAME_LENGTH;
            _liveInput.erase(_liveInput.end() - sub, _liveInput.end());
        }
    }
    else
    {
        std::vector<KeyboardEvent*>& keyboardEvents = INPUT_MGR.getKeyboardEvents();
        for (std::vector<KeyboardEvent *>::iterator i = keyboardEvents.begin(); i != keyboardEvents.end(); ++i)
        {
            KeyboardEvent& e = *(*i);
            
            switch (e._key)
            {
                case GOW_KEY_E:
                    _inputState = e.isDown() ? MIMS_ENTER_STUDIO : MIMS_NONE;
                    continue;
                case GOW_KEY_S:
                    _inputState = e.isDown() ? MIMS_START_SERVER : MIMS_NONE;
                    continue;
                case GOW_KEY_J:
                    _inputState = e.isDown() ? MIMS_JOIN_LOCAL_SERVER : MIMS_NONE;
                    continue;
                case GOW_KEY_ESCAPE:
                    _inputState = e.isDown() ? MIMS_ESCAPE : MIMS_NONE;
                    continue;
                default:
                    continue;
            }
        }
        
        std::vector<GamepadEvent*>& gamepadEvents = INPUT_MGR.getGamepadEvents();
        for (std::vector<GamepadEvent *>::iterator i = gamepadEvents.begin(); i != gamepadEvents.end(); ++i)
        {
            if (!(*i)->isPressed())
            {
                continue;
            }
            
            switch ((*i)->_type)
            {
                case GamepadEventType_BUTTON_START:
                    _inputState = MIMS_START_SERVER;
                    continue;
                case GamepadEventType_BUTTON_SELECT:
                    _inputState = MIMS_ESCAPE;
                    continue;
                default:
                    continue;
            }
        }
        
#if IS_MOBILE
        std::vector<CursorEvent*>& cursorEvents = INPUT_MGR.getCursorEvents();
        for (std::vector<CursorEvent *>::iterator i = cursorEvents.begin(); i != cursorEvents.end(); ++i)
        {
            if ((*i)->_type == CursorEventType_UP)
            {
                _inputState = MIMS_START_SERVER;
            }
            else
            {
                continue;
            }
        }
#endif
    }
}

void MainInputManager::setLiveInputMode(bool isLiveMode)
{
    _isLiveMode = isLiveMode;
    
    if (_isLiveMode)
    {
        _liveInput.clear();
    }
}

bool MainInputManager::isLiveMode()
{
    return _isLiveMode;
}

bool MainInputManager::isTimeToProcessInput()
{
    return _isTimeToProcessInput;
}

void MainInputManager::onInputProcessed()
{
    _liveInput.clear();
    _isTimeToProcessInput = false;
}

int MainInputManager::getMenuState()
{
    return _inputState;
}

std::string& MainInputManager::getLiveInputRef()
{
    return _liveInput;
}

std::string MainInputManager::getLiveInput()
{
    return _liveInput;
}

MainInputManager::MainInputManager() :
_inputState(MIMS_NONE),
_isLiveMode(false),
_isTimeToProcessInput(false)
{
    // Empty
}

MainInputManager::~MainInputManager()
{
    // Empty
}
