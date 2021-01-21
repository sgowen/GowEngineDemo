//
//  Renderer.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 4/23/20.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "TextureManager.hpp"
#include "ShaderManager.hpp"
#include "SpriteBatcher.hpp"
#include "ScreenRenderer.hpp"
#include "Framebuffer.hpp"
#include "ShaderInput.hpp"

class Renderer
{
public:
    Renderer();
    ~Renderer();
    
    void createDeviceDependentResources();
    void onWindowSizeChanged(int screenWidth, int screenHeight);
    void releaseDeviceDependentResources();
    void render();
    
private:
    TextureManager _textureManager;
    ShaderManager _shaderManager;
    SpriteBatcher _spriteBatcher;
    ScreenRenderer _screenRenderer;
    Framebuffer _framebuffer;
    mat4 _matrix;
};
