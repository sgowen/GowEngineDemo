//
//  StudioRenderer.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/4/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include <string>
#include <deque>
#include <vector>
#include <map>

#include "Constants.hpp"
#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "SpriteBatcher.hpp"
#include "LineBatcher.hpp"
#include "PolygonBatcher.hpp"
#include "ScreenRenderer.hpp"
#include "Framebuffer.hpp"
#include "Font.hpp"
#include "ShaderInput.hpp"
#include "Box2DDebugRenderer.hpp"
#include "Rektangle.hpp"

struct Shader;
class Entity;
struct TextureRegion;
struct Texture;
class World;
class StudioEngineState;
class StudioInputManager;

class StudioRenderer
{
public:
    StudioRenderer();
    ~StudioRenderer();
    
    void createDeviceDependentResources();
    void onWindowSizeChanged(int screenWidth, int screenHeight);
    void releaseDeviceDependentResources();
    void render();
    
    void update(float x, float y, float w, float h, int scale);
    void displayToast(std::string toast);
    
private:
    TextureManager _textureManager;
    ShaderManager _shaderManager;
    SpriteBatcher _fontSpriteBatcher;
    SpriteBatcher _fbSpriteBatcher;
    SpriteBatcher _activeEntitySpriteBatcher;
    SpriteBatcher _spriteBatchers[NUM_SPRITE_BATCHERS];
    std::string _textures[NUM_SPRITE_BATCHERS];
    PolygonBatcher _fillPolygonBatcher;
    LineBatcher _lineBatcher;
    Box2DDebugRenderer _box2DDebugRenderer;
    ScreenRenderer _screenRenderer;
    Font _font;
    Framebuffer _framebuffers[NUM_SPRITE_BATCHERS];
    mat4 _matrix;
    Rektangle _camBounds[NUM_CAMERAS];
    std::deque<std::string> _toasts;
    int _toastStateTime;
    int _scrollValue;
    StudioEngineState* _studioEngineState;
    StudioInputManager* _input;
    uint32_t _engineState;
    
    void renderWorld();
    void renderEntities(std::vector<Entity*>& entities);
    void endBatchWithTexture(SpriteBatcher& sb, std::string textureName, int layer);
    void renderActiveEntity();
    void renderBox2D();
    void renderGrid();
    void renderUI();
    void renderText(const char* inStr, float x, float y, int justification);
};
