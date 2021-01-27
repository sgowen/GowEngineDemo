//
//  MainRenderer.h
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include <string>

#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "SpriteBatcher.hpp"
#include "CircleBatcher.hpp"
#include "LineBatcher.hpp"
#include "PolygonBatcher.hpp"
#include "ScreenRenderer.hpp"
#include "Framebuffer.hpp"
#include "Font.hpp"
#include "ShaderInput.hpp"

class MainEngineState;

class MainRenderer
{
public:
    MainRenderer();
    ~MainRenderer();
    
    void createDeviceDependentResources();
    void onWindowSizeChanged(int screenWidth, int screenHeight);
    void releaseDeviceDependentResources();
    void render(MainEngineState& mes);
    
private:
    TextureManager _textureManager;
    ShaderManager _shaderManager;
    SpriteBatcher _spriteBatcher;
    CircleBatcher _circleBatcher;
    LineBatcher _lineBatcher;
    PolygonBatcher _fillPolygonBatcher;
    PolygonBatcher _boundsPolygonBatcher;
    ScreenRenderer _screenRenderer;
    Font _font;
    Framebuffer _framebuffer;
    mat4 _matrix;
    
    void renderMainMenu();
    void renderEnterUsernameText();
    void renderJoiningLocalServerByIPText();
    void renderText(std::string text, float x, float y, int justification);
};
