//
//  Util.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 6/15/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "Util.hpp"

#include "Entity.hpp"
#include <box2d/box2d.h>

#include "World.hpp"
#include "GowAudioEngine.hpp"
#include "PlayerController.hpp"
#include "GameEngineState.hpp"
#include "MainConfig.hpp"

void Util::handleSound(Entity* e, uint8_t fromState, uint8_t toState)
{
    if (fromState == toState)
    {
        return;
    }
    
    playSound(e->getSoundMapping(toState), e->getPosition());
}

void Util::playSound(int soundID, const b2Vec2& position)
{
    if (soundID <= 0)
    {
        return;
    }
    
    float volume = 1;
    float playerVolume = 1;
    
    World* w = GameEngineState::getInstance()->getWorld();
    assert(w);
    
    std::vector<Entity*>& players = w->getPlayers();
    for (Entity* e : players)
    {
        PlayerNetworkController* p = static_cast<PlayerNetworkController*>(e->getNetworkController());
        assert(p);
        if (!p->isLocalPlayer())
        {
            continue;
        }
        
        float distance = b2Distance(e->getPosition(), position);
        if (distance > 0)
        {
            float factor = distance / (CFG_MAIN._camWidth / 8);
            float newPlayerVolume = 1.0f / (factor * factor);
            if (newPlayerVolume < playerVolume)
            {
                playerVolume = newPlayerVolume;
            }
        }
    }
    
    volume = playerVolume;
    
    GOW_AUDIO->playSound(soundID, volume);
}

Util::Util()
{
    // Empty
}

Util::~Util()
{
    // Empty
}
