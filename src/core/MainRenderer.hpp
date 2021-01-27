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
    ScreenRenderer _screenRenderer;
    Framebuffer _framebuffer;
    Font _font;
    mat4 _matrix;
    
    void renderMainMenu();
    void renderEnterUsernameText();
    void renderJoiningLocalServerByIPText();
    void renderText(std::string text, float x, float y, int justification);
};
