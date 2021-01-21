//
//  Renderer.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 4/23/20.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "Renderer.hpp"

#include "OpenGLWrapper.hpp"
#include "Macros.hpp"
#include "Color.hpp"
#include "MathUtil.hpp"

Renderer::Renderer() :
_textureManager(),
_shaderManager(),
_spriteBatcher(64),
_screenRenderer(),
_framebuffer(2048, 1535)
{
    // Empty
}

Renderer::~Renderer()
{
    // Empty
}

void Renderer::createDeviceDependentResources()
{
    _textureManager.createDeviceDependentResources();
    _shaderManager.createDeviceDependentResources();
    _spriteBatcher.createDeviceDependentResources();
    _screenRenderer.createDeviceDependentResources();
    
    OGL.loadFramebuffer(_framebuffer, GL_LINEAR, GL_LINEAR);
}

void Renderer::onWindowSizeChanged(int screenWidth, int screenHeight)
{
    _screenRenderer.onWindowSizeChanged(screenWidth, screenHeight);
}

void Renderer::releaseDeviceDependentResources()
{
    _textureManager.releaseDeviceDependentResources();
    _shaderManager.releaseDeviceDependentResources();
    _spriteBatcher.releaseDeviceDependentResources();
    _screenRenderer.releaseDeviceDependentResources();
    
    OGL.unloadFramebuffer(_framebuffer);
}

void Renderer::render()
{
    Color white(COLOR_WHITE);
    float w = _framebuffer._width;
    float h = _framebuffer._height;
    
    mat4_identity(_matrix);
    mat4_ortho(_matrix, 0, w, 0, h, -1, 1);
    
    OGL.bindFramebuffer(_framebuffer);
    OGL.clearFramebuffer(COLOR_BLACK);
    OGL.enableBlending();
    
    {
        auto& t = _textureManager.texture("texture_002");
        auto& td = t._descriptor;
        _spriteBatcher.begin();
        _spriteBatcher.addSprite(td.textureRegion("forest"), w / 2.0f, h / 2.0f, w, h);
        _spriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, t, white);
    }
    
    float heightFactor = 1.334201954397394f;
    
    {
        auto& t = _textureManager.texture("texture_003");
        auto& td = t._descriptor;
        _spriteBatcher.begin();
        auto& chicken = td.textureRegion("obj_food_chicken");
        auto& watermelon = td.textureRegion("obj_food_watermelon");
        auto& duck = td.textureRegion("ms_duck");
        auto& pig = td.textureRegion("mr_pig");
        auto& mrbaby = td.textureRegion("mr_baby2");
        auto& dino = td.textureRegion("baby_triceratops");
        _spriteBatcher.addSprite(chicken, 400, 1000, chicken._width, chicken._height * heightFactor);
        _spriteBatcher.addSprite(watermelon, 800, 1000, watermelon._width, watermelon._height * heightFactor);
        _spriteBatcher.addSprite(duck, 400, 560, duck._width, duck._height * heightFactor);
        _spriteBatcher.addSprite(pig, 800, 560, pig._width, pig._height * heightFactor);
        _spriteBatcher.addSprite(mrbaby, 1300, 560, mrbaby._width, mrbaby._height * heightFactor);
        _spriteBatcher.addSprite(dino, 1800, 560, dino._width, dino._height * heightFactor);
        _spriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, t, white);
    }
    
    {
        auto& t = _textureManager.texture("texture_001");
        auto& td = t._descriptor;
        _spriteBatcher.begin();
        auto& tr = td.textureRegion("forest_grass2");
        _spriteBatcher.addSprite(tr, tr._width / 2.0f, tr._height / 2.0f, tr._width, tr._height);
        _spriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, t, white);
    }
    
    {
        auto& t = _textureManager.texture("texture_001");
        auto& td = t._descriptor;
        auto& menu_toys_tab = td.textureRegion("menu_toys_tab");
        auto& menu_food_tab = td.textureRegion("menu_food_tab");
        auto& menu_characters_tab = td.textureRegion("menu_characters_tab");
        _spriteBatcher.begin();
        _spriteBatcher.addSprite(menu_toys_tab, 24 + menu_characters_tab._width + menu_food_tab._width + menu_toys_tab._width / 2.0f, menu_toys_tab._height * heightFactor / 2.0f, menu_toys_tab._width, menu_toys_tab._height * heightFactor);
        _spriteBatcher.addSprite(menu_food_tab, 24 + menu_characters_tab._width + menu_food_tab._width / 2.0f, menu_food_tab._height * heightFactor / 2, menu_food_tab._width, menu_food_tab._height * heightFactor);
        _spriteBatcher.addSprite(menu_characters_tab, 24 + menu_characters_tab._width / 2.0f, menu_characters_tab._height * heightFactor / 2.0f, menu_characters_tab._width, menu_characters_tab._height * heightFactor);
        _spriteBatcher.addSprite(td.textureRegion("btn_forest"), 840 / 2.0f + 24, h - (159 / 2.0) - 24, 840, 159.0f);
        _spriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, t, white);
    }
    
    _screenRenderer.render(_shaderManager.shader("shader_001"), _framebuffer);
}
