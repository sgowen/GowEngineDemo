//
//  GameRenderer.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "Constants.hpp"

#include <string>
#include <vector>

#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "SpriteBatcher.hpp"
#include "LineBatcher.hpp"
#include "PolygonBatcher.hpp"
#include "ScreenRenderer.hpp"
#include "LightRenderer.hpp"
#include "Framebuffer.hpp"
#include "Font.hpp"
#include "ShaderInput.hpp"
#include "Box2DDebugRenderer.hpp"
#include "Rektangle.hpp"

class Entity;
class GameEngineState;

struct LightDef
{
    float _lightPosX;
    float _lightPosY;
    float _lightColorR;
    float _lightColorG;
    float _lightColorB;
    float _lightColorA;
    
    LightDef(float lightPosX, float lightPosY, float lightColorR, float lightColorG, float lightColorB, float lightColorA) : _lightPosX(lightPosX), _lightPosY(lightPosY), _lightColorR(lightColorR), _lightColorG(lightColorG), _lightColorB(lightColorB), _lightColorA(lightColorA) {}
};

class GameRenderer
{
public:
    GameRenderer();
    ~GameRenderer();
    
    void createDeviceDependentResources();
    void onWindowSizeChanged(int screenWidth, int screenHeight);
    void releaseDeviceDependentResources();
    void render();
    
private:
    TextureManager _textureManager;
    ShaderManager _shaderManager;
    SpriteBatcher _spriteBatchers[NUM_SPRITE_BATCHERS];
    std::string _textures[NUM_SPRITE_BATCHERS];
    std::string _normals[NUM_SPRITE_BATCHERS];
    SpriteBatcher _staticFontSpriteBatcher;
    SpriteBatcher _dynamicFontSpriteBatcher;
    Box2DDebugRenderer _box2DDebugRenderer;
    LightRenderer _lightRenderer;
    ScreenRenderer _screenRenderer;
    Font _font;
    Framebuffer _framebuffers[NUM_SPRITE_BATCHERS];
    mat4 _matrix;
    Rektangle _camBounds[NUM_CAMERAS];
    GameEngineState* _gameEngineState;
    uint32_t _map;
    uint32_t _engineState;
    std::vector<LightDef> _playerLights;
    std::vector<LightDef> _lights;
    
    void updateCamera();
    void renderWorld();
    void renderEntities(std::vector<Entity*>& entities);
    void renderLighting();
    void endBatchWithTexture(SpriteBatcher& sb, std::string textureName, int layer);
    void renderBox2D();
    void renderUI();
    void renderText(SpriteBatcher& sb, const char* inStr, float x, float y, int justification);
};
