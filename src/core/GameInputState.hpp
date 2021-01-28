//
//  GameInputState.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 5/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include <stdint.h>

#include "InputState.hpp"

enum GameInputStateFlags
{
    GameInputStateFlag_MovingRight = 1 << 0,
    GameInputStateFlag_MovingLeft =  1 << 1,
    GameInputStateFlag_Jumping =     1 << 2,
    GameInputStateFlag_MainAction =  1 << 3
};

class OutputMemoryBitStream;
class InputMemoryBitStream;

class GameInputState : public InputState
{    
    friend class GameInputManager;
    
public:
    class PlayerInputState
    {
        friend class GameInputState;
        friend class GameInputManager;
        
    public:
        void write(OutputMemoryBitStream& inOutputStream) const;
        void read(InputMemoryBitStream& inInputStream);
        uint8_t getInputState();
        
    private:
        uint8_t _playerID;
        uint8_t _inputState;
        
        PlayerInputState();
    };
    
    GameInputState();
    
    virtual void write(OutputMemoryBitStream& inOutputStream) const;
    virtual void read(InputMemoryBitStream& inInputStream);
    virtual void reset();
    virtual bool isEqual(InputState* inInputState) const;
    virtual void copyTo(InputState* inInputState) const;
    
    void activateNextPlayer(uint8_t playerID);
    PlayerInputState* getPlayerInputStateForID(uint8_t playerID);
    bool isRequestingToAddLocalPlayer() const;
    
private:
    PlayerInputState _playerInputStates[4];
    
    PlayerInputState& getPlayerInputState(int index);
};
