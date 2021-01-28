//
//  Util.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 6/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include <stdint.h>

class Entity;
struct b2Vec2;

class Util
{
public:
    static void handleSound(Entity* e, uint8_t fromState, uint8_t toState);
    static void playSound(int soundID, const b2Vec2& position);
    
    Util();
    ~Util();
    Util(const Util&);
    Util& operator=(const Util&);
};
