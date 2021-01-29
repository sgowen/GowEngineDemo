//
//  GameInputManager.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 5/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "GameInputManager.hpp"

#include "GameInputState.hpp"
#include "Move.hpp"
#include "GameEngineState.hpp"

#include "Timing.hpp"
#include "InputManager.hpp"
#include "Constants.hpp"
#include "KeyboardLookup.hpp"
#include "StringUtil.hpp"
#include "MathUtil.hpp"
#include "Constants.hpp"
#include "GowAudioEngine.hpp"
#include "Server.hpp"
#include "NetworkManagerClient.hpp"
#include "CursorConverter.hpp"
#include "Macros.hpp"
#include "MainConfig.hpp"
#include "Vector2.hpp"

#include <sstream>

GameInputManager* GameInputManager::s_instance = NULL;

void GameInputManager::create()
{
    assert(!s_instance);
    
    s_instance = new GameInputManager();
}

GameInputManager * GameInputManager::getInstance()
{
    return s_instance;
}

void GameInputManager::destroy()
{
    assert(s_instance);
    
    delete s_instance;
    s_instance = NULL;
}

void GameInputManager::sRemoveProcessedMoves(float inLastMoveProcessedOnServerTimestamp)
{
    getInstance()->getMoveList().removeProcessedMoves(inLastMoveProcessedOnServerTimestamp, GameInputManager::sHandleInputStateRelease);
}

MoveList& GameInputManager::sGetMoveList()
{
    return getInstance()->getMoveList();
}

void GameInputManager::sOnPlayerWelcomed(uint8_t playerID)
{
    getInstance()->_currentState->activateNextPlayer(playerID);
}

void GameInputManager::sHandleInputStateRelease(InputState* inputState)
{
    GameInputState* mainInputState = static_cast<GameInputState*>(inputState);
    getInstance()->_inputStates.free(mainInputState);
}

void GameInputManager::update()
{
    _inputState = GIMS_NONE;
    
    std::vector<KeyboardEvent*>& keyboardEvents = INPUT_MGR.getKeyboardEvents();
    for (std::vector<KeyboardEvent *>::iterator i = keyboardEvents.begin(); i != keyboardEvents.end(); ++i)
    {
        KeyboardEvent& e = *(*i);
        switch (e._key)
        {
            case GOW_KEY_M:
                if (e.isDown())
                {
                    GOW_AUDIO->setMusicDisabled(!GOW_AUDIO->isMusicDisabled());
                }
                continue;
            case GOW_KEY_N:
                if (e.isDown())
                {
                    GOW_AUDIO->setSoundsDisabled(!GOW_AUDIO->areSoundsDisabled());
                }
                continue;
            case GOW_KEY_B:
                _gameEngineState->_state ^= e.isDown() ? GESS_DisplayBox2D : 0;
                continue;
            case GOW_KEY_I:
                _gameEngineState->_state ^= e.isDown() ? GESS_Interpolation : 0;
                continue;
            case GOW_KEY_L:
                _gameEngineState->_state ^= e.isDown() ? GESS_Lighting : 0;
                continue;
            case GOW_KEY_T:
                if (e.isDown() && Server::getInstance())
                {
                    Server::getInstance()->toggleMap();
                }
                continue;
            case GOW_KEY_U:
                _gameEngineState->_state ^= e.isDown() ? GESS_DisplayUI : 0;
                continue;
            case GOW_KEY_ARROW_UP:
                _playerLightZDelta = e.isPressed() ? 0.001f : 0;
                continue;
            case GOW_KEY_ARROW_DOWN:
                _playerLightZDelta = e.isPressed() ? -0.001f : 0;
                continue;
            case GOW_KEY_ESCAPE:
                if (e.isDown())
                {
                    dropPlayer(0);
                }
                continue;
            default:
            {
                switch (e._key)
                {
                    // Player 1
                    case GOW_KEY_W:
                        SET_BIT(_currentState->getPlayerInputState(0)._inputState, GameInputStateFlag_Jumping, e.isPressed());
                        continue;
                    case GOW_KEY_A:
                        SET_BIT(_currentState->getPlayerInputState(0)._inputState, GameInputStateFlag_MovingLeft, e.isPressed());
                        continue;
                    case GOW_KEY_D:
                        SET_BIT(_currentState->getPlayerInputState(0)._inputState, GameInputStateFlag_MovingRight, e.isPressed());
                        continue;
                    case GOW_KEY_SPACE_BAR:
                        SET_BIT(_currentState->getPlayerInputState(0)._inputState, GameInputStateFlag_MainAction, e.isPressed());
                        continue;
#if IS_DEBUG
                    // Add local players, debug Only
                    case GOW_KEY_TWO:
                        SET_BIT(_currentState->getPlayerInputState(1)._inputState, GameInputStateFlag_Jumping, e.isPressed());
                        continue;
                    case GOW_KEY_THREE:
                        SET_BIT(_currentState->getPlayerInputState(2)._inputState, GameInputStateFlag_Jumping, e.isPressed());
                        continue;
                    case GOW_KEY_FOUR:
                        SET_BIT(_currentState->getPlayerInputState(3)._inputState, GameInputStateFlag_Jumping, e.isPressed());
                        continue;
                    case GOW_KEY_ARROW_LEFT:
                        SET_BIT(_currentState->getPlayerInputState(1)._inputState, GameInputStateFlag_MovingLeft, e.isPressed());
                        SET_BIT(_currentState->getPlayerInputState(2)._inputState, GameInputStateFlag_MovingLeft, e.isPressed());
                        SET_BIT(_currentState->getPlayerInputState(3)._inputState, GameInputStateFlag_MovingLeft, e.isPressed());
                        continue;
                    case GOW_KEY_ARROW_RIGHT:
                        SET_BIT(_currentState->getPlayerInputState(1)._inputState, GameInputStateFlag_MovingRight, e.isPressed());
                        SET_BIT(_currentState->getPlayerInputState(2)._inputState, GameInputStateFlag_MovingRight, e.isPressed());
                        SET_BIT(_currentState->getPlayerInputState(3)._inputState, GameInputStateFlag_MovingRight, e.isPressed());
                        continue;
                    case GOW_KEY_PERIOD:
                        SET_BIT(_currentState->getPlayerInputState(1)._inputState, GameInputStateFlag_MainAction, e.isPressed());
                        SET_BIT(_currentState->getPlayerInputState(2)._inputState, GameInputStateFlag_MainAction, e.isPressed());
                        SET_BIT(_currentState->getPlayerInputState(3)._inputState, GameInputStateFlag_MainAction, e.isPressed());
                        continue;
                    case GOW_KEY_SEVEN:
                        if (e.isDown())
                        {
                            dropPlayer(1);
                        }
                        continue;
                    case GOW_KEY_EIGHT:
                        if (e.isDown())
                        {
                            dropPlayer(2);
                        }
                        continue;
                    case GOW_KEY_NINE:
                        if (e.isDown())
                        {
                            dropPlayer(3);
                        }
                        continue;
#endif
                    default:
                        continue;
                }
            }
        }
    }
    
    bool isMovingRight[4] = {false};
    bool isMovingLeft[4] = {false};
    bool isAction[4] = {false};
    
    std::vector<GamepadEvent*>& gamepadEvents = INPUT_MGR.getGamepadEvents();
    for (std::vector<GamepadEvent *>::iterator i = gamepadEvents.begin(); i != gamepadEvents.end(); ++i)
    {
        GamepadEvent& e = *(*i);
        switch (e._type)
        {
            case GamepadEventType_BUTTON_A:
                SET_BIT(_currentState->getPlayerInputState(e._index)._inputState, GameInputStateFlag_Jumping, e.isPressed());
                continue;
            case GamepadEventType_D_PAD_RIGHT:
            {
                if (!isMovingRight[e._index])
                {
                    SET_BIT(_currentState->getPlayerInputState(e._index)._inputState, GameInputStateFlag_MovingRight, e.isPressed());
                    isMovingRight[e._index] = _currentState->getPlayerInputState(e._index)._inputState & GameInputStateFlag_MovingRight;
                }
            }
                continue;
            case GamepadEventType_D_PAD_LEFT:
            {
                if (!isMovingLeft[e._index])
                {
                    SET_BIT(_currentState->getPlayerInputState(e._index)._inputState, GameInputStateFlag_MovingLeft, e.isPressed());
                    isMovingLeft[e._index] = _currentState->getPlayerInputState(e._index)._inputState & GameInputStateFlag_MovingLeft;
                }
            }
                continue;
            case GamepadEventType_STICK_LEFT:
            {
                float val = sanitizeCloseToZeroValue(e._x);
                if (!isMovingRight[e._index])
                {
                    SET_BIT(_currentState->getPlayerInputState(e._index)._inputState, GameInputStateFlag_MovingRight, val > 0);
                    
                    isMovingRight[e._index] = _currentState->getPlayerInputState(e._index)._inputState & GameInputStateFlag_MovingRight;
                }
                
                if (!isMovingLeft[e._index])
                {
                    SET_BIT(_currentState->getPlayerInputState(e._index)._inputState, GameInputStateFlag_MovingLeft, val < 0);
                    isMovingLeft[e._index] = _currentState->getPlayerInputState(e._index)._inputState & GameInputStateFlag_MovingLeft;
                }
            }
                continue;
            case GamepadEventType_BUTTON_X:
            case GamepadEventType_TRIGGER:
                if (!isAction[e._index])
                {
                    SET_BIT(_currentState->getPlayerInputState(e._index)._inputState, GameInputStateFlag_MainAction, e._x > 0 || e._y > 0);
                    isAction[e._index] = _currentState->getPlayerInputState(e._index)._inputState & GameInputStateFlag_MainAction;
                }
                continue;
            case GamepadEventType_BUTTON_SELECT:
                if (e.isPressed())
                {
                    dropPlayer(e._index);
                }
                continue;
            default:
                continue;
        }
    }
    
//#if IS_MOBILE
    std::vector<CursorEvent*>& cursorEvents = INPUT_MGR.getCursorEvents();
    for (std::vector<CursorEvent *>::iterator i = cursorEvents.begin(); i != cursorEvents.end(); ++i)
    {
        CursorEvent& e = *(*i);
        if (e._type == CursorEventType_DOWN
            || e._type == CursorEventType_DRAGGED)
        {
            Vector2& vec = CURSOR_CONVERTER.convert(*(*i));
            SET_BIT(_currentState->getPlayerInputState(0)._inputState, GameInputStateFlag_MovingLeft, vec.x() < (CFG_MAIN._camWidth / 2));
            SET_BIT(_currentState->getPlayerInputState(0)._inputState, GameInputStateFlag_MovingRight, vec.x() > (CFG_MAIN._camWidth / 2));
            
            if (e._type == CursorEventType_DOWN)
            {
                SET_BIT(_currentState->getPlayerInputState(0)._inputState, GameInputStateFlag_Jumping, true);
            }
        }
        else
        {
            _currentState->getPlayerInputState(0)._inputState = 0;
            
            continue;
        }
    }
//#endif
    
    if (_currentState->isRequestingToAddLocalPlayer())
    {
        NW_MGR_CLIENT->requestToAddLocalPlayer();
    }
    
    CFG_MAIN._playerLightZ = clamp(CFG_MAIN._playerLightZ + _playerLightZDelta, -0.1f, 0.3f);
    _pendingMove = &sampleInputAsMove();
}

const Move* GameInputManager::getPendingMove()
{
    return _pendingMove;
}

void GameInputManager::clearPendingMove()
{
    _pendingMove = NULL;
}

GameInputState* GameInputManager::getInputState()
{
    return _currentState;
}

MoveList& GameInputManager::getMoveList()
{
    return _moveList;
}

int GameInputManager::getMenuState()
{
    return _inputState;
}

const Move& GameInputManager::sampleInputAsMove()
{
    GameInputState* inputState = _inputStates.obtain();
    _currentState->copyTo(inputState);
    
    return _moveList.addMove(inputState, _gameEngineState->_timing->getTime());
}

void GameInputManager::dropPlayer(int index)
{
    _currentState->getPlayerInputState(index)._playerID = INPUT_UNASSIGNED;
    
    if (index > 0)
    {
        NW_MGR_CLIENT->requestToDropLocalPlayer(1);
    }
    else
    {
        _inputState = GIMS_ESCAPE;
    }
}

GameInputManager::GameInputManager() :
_currentState(_inputStates.obtain()),
_pendingMove(NULL),
_gameEngineState(GameEngineState::getInstance()),
_inputState(GIMS_NONE),
_isTimeToProcessInput(false),
_playerLightZDelta(0)
{
    // Empty
}

GameInputManager::~GameInputManager()
{
    _inputStates.free(_currentState);
}
