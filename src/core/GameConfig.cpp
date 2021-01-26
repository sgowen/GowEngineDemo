//
//  GameConfig.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/5/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "GameConfig.hpp"

#include "Config.hpp"

GameConfig& GameConfig::getInstance()
{
    static GameConfig ret = GameConfig();
    return ret;
}

void GameConfig::init()
{
    Config c;
    c.initWithJSONFile("config.json");
    
    _serverPort = c.getInt("ServerPort");
    _clientPort = c.getInt("ClientPort");
    _framebufferWidth = c.getInt("FramebufferWidth");
    _framebufferHeight = c.getInt("FramebufferHeight");
    _camWidth = c.getInt("CamWidth");
    _camHeight = c.getInt("CamHeight");
    _parallaxLayer0FactorX = c.getDouble("ParallaxLayer0FactorX");
    _parallaxLayer0FactorY = c.getDouble("ParallaxLayer0FactorY");
    _parallaxLayer1FactorX = c.getDouble("ParallaxLayer1FactorX");
    _parallaxLayer1FactorY = c.getDouble("ParallaxLayer1FactorY");
    _parallaxLayer2FactorX = c.getDouble("ParallaxLayer2FactorX");
    _parallaxLayer2FactorY = c.getDouble("ParallaxLayer2FactorY");
    _gravity = c.getFloat("Gravity");
    _maxCrawlerVelocityX = c.getFloat("MaxCrawlerVelocityX");
    _robotLightPositionFactorY = c.getFloat("RobotLightPositionFactorY");
    _maxRobotVelocityX = c.getFloat("MaxRobotVelocityX");
    _maxRobotVelocityY = c.getFloat("MaxRobotVelocityY");
    _playerLightZ = c.getFloat("PlayerLightZ");
    _behindPlayerLightZFactor = c.getFloat("BehindPlayerLightZFactor");
    _frontPlayerLightZFactor = c.getFloat("FrontPlayerLightZFactor");
    _playerLightColor[0] = c.getFloat("PlayerLightColorR");
    _playerLightColor[1] = c.getFloat("PlayerLightColorG");
    _playerLightColor[2] = c.getFloat("PlayerLightColorB");
    _playerLightColor[3] = c.getFloat("PlayerLightColorA");
    _ambientColor[0] = c.getFloat("AmbientColorR");
    _ambientColor[1] = c.getFloat("AmbientColorG");
    _ambientColor[2] = c.getFloat("AmbientColorB");
    _ambientColor[3] = c.getFloat("AmbientColorA");
    _fallOff[0] = c.getFloat("LightFalloffX");
    _fallOff[1] = c.getFloat("LightFalloffY");
    _fallOff[2] = c.getFloat("LightFalloffZ");
    _tempStaticLight1[0] = c.getFloat("TempStaticLight1X");
    _tempStaticLight1[1] = c.getFloat("TempStaticLight1Y");
    _tempStaticLight1[2] = c.getFloat("TempStaticLight1R");
    _tempStaticLight1[3] = c.getFloat("TempStaticLight1G");
    _tempStaticLight1[4] = c.getFloat("TempStaticLight1B");
    _tempStaticLight1[5] = c.getFloat("TempStaticLight1A");
    _tempStaticLight2[0] = c.getFloat("TempStaticLight2X");
    _tempStaticLight2[1] = c.getFloat("TempStaticLight2Y");
    _tempStaticLight2[2] = c.getFloat("TempStaticLight2R");
    _tempStaticLight2[3] = c.getFloat("TempStaticLight2G");
    _tempStaticLight2[4] = c.getFloat("TempStaticLight2B");
    _tempStaticLight2[5] = c.getFloat("TempStaticLight2A");
}

GameConfig::GameConfig()
{
    // Empty
}

GameConfig::~GameConfig()
{
    // Empty
}
