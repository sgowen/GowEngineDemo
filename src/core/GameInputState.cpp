//
//  GameInputState.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 5/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "GameInputState.hpp"

#include "OutputMemoryBitStream.hpp"
#include "InputMemoryBitStream.hpp"

#include "Constants.hpp"

#include <assert.h>

GameInputState::GameInputState() : InputState()
{
    // Empty
}

void GameInputState::write(OutputMemoryBitStream& inOutputStream) const
{
    for (int i = 0; i < MAX_NUM_PLAYERS_PER_SERVER; ++i)
    {
        bool playerID = _playerInputStates[i]._playerID != INPUT_UNASSIGNED;
        inOutputStream.write(playerID);
        if (playerID)
        {
            _playerInputStates[i].write(inOutputStream);
        }
    }
}

void GameInputState::read(InputMemoryBitStream& inInputStream)
{
    bool isInputAssignedBit;
    
    for (int i = 0; i < MAX_NUM_PLAYERS_PER_SERVER; ++i)
    {
        inInputStream.read(isInputAssignedBit);
        if (isInputAssignedBit)
        {
            _playerInputStates[i].read(inInputStream);
        }
    }
}

void GameInputState::reset()
{
    for (PlayerInputState& is : _playerInputStates)
    {
        is._playerID = INPUT_UNASSIGNED;
        is._inputState = 0;
    }
}

bool GameInputState::isEqual(InputState* inInputState) const
{
    GameInputState* in = static_cast<GameInputState*>(inInputState);
    
    for (int i = 0; i < MAX_NUM_PLAYERS_PER_SERVER; ++i)
    {
        if (in->_playerInputStates[i]._playerID != _playerInputStates[i]._playerID)
        {
            return false;
        }
        
        if (in->_playerInputStates[i]._inputState != _playerInputStates[i]._inputState)
        {
            return false;
        }
    }
    
    return true;
}

void GameInputState::copyTo(InputState* inInputState) const
{
    GameInputState* inGameInputState = static_cast<GameInputState*>(inInputState);
    
    for (int i = 0; i < MAX_NUM_PLAYERS_PER_SERVER; ++i)
    {
        inGameInputState->_playerInputStates[i]._playerID = _playerInputStates[i]._playerID;
        inGameInputState->_playerInputStates[i]._inputState = _playerInputStates[i]._inputState;
    }
}

void GameInputState::activateNextPlayer(uint8_t playerID)
{
    for (int i = 0; i < MAX_NUM_PLAYERS_PER_SERVER; ++i)
    {
        if (_playerInputStates[i]._playerID == INPUT_UNASSIGNED)
        {
            _playerInputStates[i]._playerID = playerID;
            return;
        }
    }
}

GameInputState::PlayerInputState* GameInputState::getPlayerInputStateForID(uint8_t playerID)
{
    for (int i = 0; i < MAX_NUM_PLAYERS_PER_SERVER; ++i)
    {
        if (_playerInputStates[i]._playerID == playerID)
        {
            return &_playerInputStates[i];
        }
    }
    
    return NULL;
}

bool GameInputState::isRequestingToAddLocalPlayer() const
{
    for (int i = 1; i < MAX_NUM_PLAYERS_PER_SERVER; ++i)
    {
        if (_playerInputStates[i]._playerID == INPUT_UNASSIGNED
            && (_playerInputStates[i]._inputState & GameInputStateFlag_Jumping ||
                _playerInputStates[i]._inputState & GameInputStateFlag_MainAction))
        {
            return true;
        }
    }
    
    return false;
}

GameInputState::PlayerInputState& GameInputState::getPlayerInputState(int index)
{
    return _playerInputStates[index];
}

void GameInputState::PlayerInputState::write(OutputMemoryBitStream& inOutputStream) const
{
    inOutputStream.write<uint8_t, 3>(_playerID);
    inOutputStream.write<uint8_t, 5>(_inputState);
}

void GameInputState::PlayerInputState::read(InputMemoryBitStream& inInputStream)
{
    inInputStream.read<uint8_t, 3>(_playerID);
    inInputStream.read<uint8_t, 5>(_inputState);
}

uint8_t GameInputState::PlayerInputState::getInputState()
{
    return _inputState;
}

GameInputState::PlayerInputState::PlayerInputState() :
_playerID(INPUT_UNASSIGNED),
_inputState(0)
{
    // Empty
}
