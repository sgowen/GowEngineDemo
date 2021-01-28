//
//  GameInputManager.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 5/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "MoveList.hpp"
#include "Pool.hpp"
#include "GameInputState.hpp"

#include <string>

class Move;
class GameEngineState;

#define INPUT_MANAGER_CALLBACKS GameInputManager::sRemoveProcessedMoves, GameInputManager::sGetMoveList, GameInputManager::sOnPlayerWelcomed

enum GameInputManagerState
{
    GIMS_NONE = 0,
    GIMS_ESCAPE
};

class GameInputManager
{
public:
    static void create();
    static GameInputManager* getInstance();
    static void destroy();
    
    static void sRemoveProcessedMoves(float inLastMoveProcessedOnServerTimestamp);
    static MoveList& sGetMoveList();
    static void sOnPlayerWelcomed(uint8_t playerID);
    static void sHandleInputStateRelease(InputState* inputState);
    
    void update();
    const Move* getPendingMove();
    void clearPendingMove();
    GameInputState* getInputState();
    MoveList& getMoveList();
    int getMenuState();
    
private:
    static GameInputManager* s_instance;
    
    Pool<GameInputState> _inputStates;
    GameInputState* _currentState;
    MoveList _moveList;
    const Move* _pendingMove;
    int _inputState;
    bool _isTimeToProcessInput;
    GameEngineState* _gameEngineState;
    float _playerLightZDelta;
    
    const Move& sampleInputAsMove();
    
    void dropPlayer(int index);
    
    GameInputManager();
    ~GameInputManager();
    GameInputManager(const GameInputManager&);
    GameInputManager& operator=(const GameInputManager&);
};
