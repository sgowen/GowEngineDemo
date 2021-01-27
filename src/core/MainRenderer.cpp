//
//  MainRenderer.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "MainRenderer.hpp"

#include "MainEngineState.hpp"

#include "OpenGLWrapper.hpp"
#include "Macros.hpp"
#include "Color.hpp"
#include "MainConfig.hpp"
#include "MainInputManager.hpp"
#include "CursorConverter.hpp"

#include <sstream>
#include <ctime>
#include <string>
#include <assert.h>

MainRenderer::MainRenderer() :
_textureManager(),
_shaderManager(),
_spriteBatcher(128),
_circleBatcher(128),
_lineBatcher(128),
_fillPolygonBatcher(128, true),
_boundsPolygonBatcher(128, false),
_screenRenderer(),
_font(0, 0, 16, 64, 75, 1024, 1024),
_framebuffer(2048, 1535)
{
    // Empty
}

MainRenderer::~MainRenderer()
{
    // Empty
}

void MainRenderer::createDeviceDependentResources()
{
    _textureManager.createDeviceDependentResources();
    _shaderManager.createDeviceDependentResources();
    _spriteBatcher.createDeviceDependentResources();
    _circleBatcher.createDeviceDependentResources();
    _lineBatcher.createDeviceDependentResources();
    _fillPolygonBatcher.createDeviceDependentResources();
    _boundsPolygonBatcher.createDeviceDependentResources();
    _screenRenderer.createDeviceDependentResources();
    
    OGL.loadFramebuffer(_framebuffer, GL_LINEAR, GL_LINEAR);
}

void MainRenderer::onWindowSizeChanged(int screenWidth, int screenHeight)
{
    _screenRenderer.onWindowSizeChanged(screenWidth, screenHeight);
}

void MainRenderer::releaseDeviceDependentResources()
{
    _textureManager.releaseDeviceDependentResources();
    _shaderManager.releaseDeviceDependentResources();
    _spriteBatcher.releaseDeviceDependentResources();
    _circleBatcher.releaseDeviceDependentResources();
    _lineBatcher.releaseDeviceDependentResources();
    _fillPolygonBatcher.releaseDeviceDependentResources();
    _boundsPolygonBatcher.releaseDeviceDependentResources();
    _screenRenderer.releaseDeviceDependentResources();
    
    OGL.unloadFramebuffer(_framebuffer);
}

void MainRenderer::render(MainEngineState& mes)
{
    Color white(COLOR_WHITE);
    
    mat4_identity(_matrix);
    mat4_ortho(_matrix, 0, MAIN_CFG._camWidth, 0, MAIN_CFG._camHeight, -1, 1);
    CURSOR_CONVERTER.setMatrixSize(MAIN_CFG._camWidth, MAIN_CFG._camHeight);
    
    OGL.bindFramebuffer(_framebuffer);
    OGL.clearFramebuffer(COLOR_BLACK);
    OGL.enableBlending();
    
    // TEMP BEGIN geometry testing
    Color r(COLOR_RED);
    _lineBatcher.begin();
    _lineBatcher.addLine(5, 5, 15, 15);
    _lineBatcher.addLine(25, 25, 18, 21);
    _lineBatcher.end(_shaderManager.shader("shader_003"), _matrix, r);
    
    Color g(COLOR_GREEN);
    _circleBatcher.begin();
    _circleBatcher.addCircle(8, 32, 3);
    _circleBatcher.addPartialCircle(6, 18, 2, 90);
    _circleBatcher.end(_shaderManager.shader("shader_003"), _matrix, g);
    
    Color b(COLOR_BLUE);
    _fillPolygonBatcher.begin();
    _fillPolygonBatcher.addRectangle(40, 20, 46, 24);
    _fillPolygonBatcher.addTriangle(42, 4, 46, 10, 50, 4);
    _fillPolygonBatcher.end(_shaderManager.shader("shader_003"), _matrix, b);
    
    _boundsPolygonBatcher.begin();
    _boundsPolygonBatcher.addRectangle(48, 24, 56, 28);
    _boundsPolygonBatcher.addTriangle(3, 2, 6, 4, 9, 2);
    _boundsPolygonBatcher.end(_shaderManager.shader("shader_003"), _matrix, b);
    // TEMP END geometry testing
    
    _spriteBatcher.begin();
    switch (mes._state)
    {
        case MESS_Default:
            renderMainMenu();
            break;
        case MESS_InputName:
            renderEnterUsernameText();
            break;
        case MESS_InputIpAddress:
            renderJoiningLocalServerByIPText();
            break;
        default:
            break;
    }
    _spriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, _textureManager.texture("texture_010"), white);

    _screenRenderer.render(_shaderManager.shader("shader_001"), _framebuffer);
}

void MainRenderer::renderMainMenu()
{
    renderText("[E] to enter Studio",       MAIN_CFG._camWidth / 4, MAIN_CFG._camHeight - 2, FONT_ALIGN_LEFT);
    renderText("[S] to start local server", MAIN_CFG._camWidth / 4, MAIN_CFG._camHeight - 6, FONT_ALIGN_LEFT);
    renderText("[J] to join server by IP",  MAIN_CFG._camWidth / 4, MAIN_CFG._camHeight - 8, FONT_ALIGN_LEFT);
    renderText("[ESC] to exit game",        MAIN_CFG._camWidth / 2,                       4, FONT_ALIGN_CENTER);
}

void MainRenderer::renderEnterUsernameText()
{
    renderText("Enter Username to join, [ESC] to exit", MAIN_CFG._camWidth / 2, MAIN_CFG._camHeight - 4, FONT_ALIGN_CENTER);
    renderText(MainInputManager::getInstance()->getLiveInputRef().c_str(), MAIN_CFG._camWidth / 2, MAIN_CFG._camHeight - 8, FONT_ALIGN_CENTER);
}

void MainRenderer::renderJoiningLocalServerByIPText()
{
    renderText("Enter Server Address to join, [ESC] to exit", MAIN_CFG._camWidth / 2, MAIN_CFG._camHeight - 4, FONT_ALIGN_CENTER);
    renderText(MainInputManager::getInstance()->getLiveInputRef().c_str(), MAIN_CFG._camWidth / 2, MAIN_CFG._camHeight - 8, FONT_ALIGN_CENTER);
}

void MainRenderer::renderText(std::string text, float x, float y, int justification)
{
    float glyphWidth = 64 / MAIN_CFG._camWidth;
    float glyphHeight = glyphWidth * 1.171875f;

    _font.renderText(_spriteBatcher, text, x, y, glyphWidth, glyphHeight, justification);
}
