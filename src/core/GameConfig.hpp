//
//  GameConfig.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/5/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#define GAME_CONFIG GameConfig::getInstance()

class GameConfig
{
public:
    static GameConfig& getInstance();
    
    int _serverPort;
    int _clientPort;
    int _framebufferWidth;
    int _framebufferHeight;
    int _camWidth;
    int _camHeight;
    double _parallaxLayer0FactorX;
    double _parallaxLayer0FactorY;
    double _parallaxLayer1FactorX;
    double _parallaxLayer1FactorY;
    double _parallaxLayer2FactorX;
    double _parallaxLayer2FactorY;
    float _gravity;
    float _maxCrawlerVelocityX;
    float _robotLightPositionFactorY;
    float _maxRobotVelocityX;
    float _maxRobotVelocityY;
    float _playerLightZ;
    float _behindPlayerLightZFactor;
    float _frontPlayerLightZFactor;
    float _playerLightColor[4];
    float _ambientColor[4];
    float _fallOff[3];
    float _tempStaticLight1[6];
    float _tempStaticLight2[6];
    
    void init();
    
private:
    GameConfig();
    ~GameConfig();
    GameConfig(const GameConfig&);
    GameConfig& operator=(const GameConfig&);
};
