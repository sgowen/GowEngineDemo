//
//  StudioRenderer.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/4/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "StudioRenderer.hpp"

#include "TextureManager.hpp"
#include "Font.hpp"
#include "World.hpp"
#include "SpriteBatcher.hpp"
#include "PolygonBatcher.hpp"
#include "LineBatcher.hpp"
#include "CircleBatcher.hpp"
#include "Shader.hpp"
#include "Entity.hpp"
#include "TextureRegion.hpp"
#include "Texture.hpp"
#include "Box2DDebugRenderer.hpp"
#include "StudioEngineState.hpp"
#include "StudioInputManager.hpp"

#include "Color.hpp"
#include "Assets.hpp"
#include "Constants.hpp"
#include "Rektangle.hpp"
#include "Line.hpp"
#include "SpriteBatcher.hpp"
#include "TextureRegion.hpp"
#include "Macros.hpp"
#include "StringUtil.hpp"
#include "GowUtil.hpp"
#include "MathUtil.hpp"
#include "FPSUtil.hpp"
#include "Circle.hpp"
#include "CircleBatcher.hpp"
#include "PolygonBatcher.hpp"
#include "LineBatcher.hpp"
#include "Constants.hpp"
#include "Circle.hpp"
#include "Line.hpp"
#include "Assets.hpp"
#include "CursorConverter.hpp"
#include "EntityLayoutMapper.hpp"
#include "EntityMapper.hpp"
#include "Config.hpp"
#include "StringUtil.hpp"
#include "MainConfig.hpp"
#include "OpenGLWrapper.hpp"

#include <sstream>
#include <ctime>
#include <assert.h>
#include <cfloat>

StudioRenderer::StudioRenderer() :
_textureManager(),
_shaderManager(),
_fontSpriteBatcher(4096),
_fbSpriteBatcher(4096),
_activeEntitySpriteBatcher(4096),
_spriteBatchers{
    SpriteBatcher(4096),
    SpriteBatcher(4096),
    SpriteBatcher(4096),
    SpriteBatcher(4096),
    SpriteBatcher(4096),
    SpriteBatcher(4096),
    SpriteBatcher(4096),
    SpriteBatcher(4096),
    SpriteBatcher(4096),
    SpriteBatcher(4096)
},
_fillPolygonBatcher(4096, true),
_lineBatcher(4096),
_box2DDebugRenderer(4096),
_screenRenderer(),
_font(0, 0, 16, 64, 75, 1024, 1024),
_framebuffers{
    Framebuffer(1024, 576),
    Framebuffer(1024, 576),
    Framebuffer(1024, 576),
    Framebuffer(1024, 576),
    Framebuffer(1024, 576),
    Framebuffer(1024, 576),
    Framebuffer(1024, 576),
    Framebuffer(1024, 576),
    Framebuffer(1024, 576),
    Framebuffer(1024, 576)
},
_camBounds{
    Rektangle(0, 0, CFG_MAIN._camWidth, CFG_MAIN._camHeight),
    Rektangle(0, 0, CFG_MAIN._camWidth, CFG_MAIN._camHeight),
    Rektangle(0, 0, CFG_MAIN._camWidth, CFG_MAIN._camHeight),
    Rektangle(0, 0, CFG_MAIN._camWidth, CFG_MAIN._camHeight)
},
_toastStateTime(0),
_scrollValue(1),
_studioEngineState(NULL),
_input(NULL),
_engineState(0)
{
    // Empty
}

StudioRenderer::~StudioRenderer()
{
    // Empty
}

void StudioRenderer::createDeviceDependentResources()
{
    _studioEngineState = StudioEngineState::getInstance();
    _input = StudioInputManager::getInstance();
    
    _textureManager.createDeviceDependentResources();
    _shaderManager.createDeviceDependentResources();
    _fontSpriteBatcher.createDeviceDependentResources();
    _fbSpriteBatcher.createDeviceDependentResources();
    _activeEntitySpriteBatcher.createDeviceDependentResources();
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        _spriteBatchers[i].createDeviceDependentResources();
    }
    
    _fillPolygonBatcher.createDeviceDependentResources();
    _lineBatcher.createDeviceDependentResources();
    _box2DDebugRenderer.createDeviceDependentResources();
    _screenRenderer.createDeviceDependentResources();
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        OGL.loadFramebuffer(_framebuffers[i], GL_LINEAR, GL_LINEAR);
    }
}

void StudioRenderer::onWindowSizeChanged(int screenWidth, int screenHeight)
{
    _screenRenderer.onWindowSizeChanged(screenWidth, screenHeight);
}

void StudioRenderer::releaseDeviceDependentResources()
{
    _textureManager.releaseDeviceDependentResources();
    _shaderManager.releaseDeviceDependentResources();
    _fontSpriteBatcher.releaseDeviceDependentResources();
    _fbSpriteBatcher.releaseDeviceDependentResources();
    _activeEntitySpriteBatcher.releaseDeviceDependentResources();
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        _spriteBatchers[i].releaseDeviceDependentResources();
    }
    
    _fillPolygonBatcher.releaseDeviceDependentResources();
    _lineBatcher.releaseDeviceDependentResources();
    _box2DDebugRenderer.releaseDeviceDependentResources();
    _screenRenderer.releaseDeviceDependentResources();
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        OGL.unloadFramebuffer(_framebuffers[i]);
    }
}

void StudioRenderer::render()
{
    _engineState = _studioEngineState->_state;

    renderWorld();
    renderActiveEntity();
    renderBox2D();
    renderGrid();
    renderUI();

    _screenRenderer.renderToScreen(_shaderManager.shader("shader_001"), _framebuffers[9]);
}

void StudioRenderer::update(float x, float y, float w, float h, int scale)
{
    _scrollValue = scale;
    
    _camBounds[3]._lowerLeft.set(x, y);
    _camBounds[2]._lowerLeft.set(x * CFG_MAIN._parallaxLayer2FactorX, y * CFG_MAIN._parallaxLayer2FactorY);
    _camBounds[1]._lowerLeft.set(x * CFG_MAIN._parallaxLayer1FactorX, y * CFG_MAIN._parallaxLayer1FactorY);
    _camBounds[0]._lowerLeft.set(x * CFG_MAIN._parallaxLayer0FactorX, y * CFG_MAIN._parallaxLayer0FactorY);
    
    for (int i = 0; i < NUM_CAMERAS; ++i)
    {
        _camBounds[i]._width = w;
        _camBounds[i]._height = h;
    }
    
    if (_toasts.size() > 0)
    {
        ++_toastStateTime;
        if (_toastStateTime >= 90)
        {
            _toasts.pop_front();
            _toastStateTime = 0;
        }
    }
}

void StudioRenderer::displayToast(std::string toast)
{
    _toasts.push_back(toast);
    _toastStateTime = 0;
}

void StudioRenderer::renderWorld()
{
    World& world = _studioEngineState->_world;
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        _spriteBatchers[i].begin();
    }
    
    if (_engineState & SESS_DisplayTypes)
    {
        _fontSpriteBatcher.begin();
    }
    
    std::fill(_textures, _textures + NUM_SPRITE_BATCHERS, "");
    
    renderEntities(world.getLayers());
    renderEntities(world.getStaticEntities());
    renderEntities(world.getDynamicEntities());
    
    OGL.enableBlending(true);
    OGL.bindFramebuffer(_framebuffers[0]);
    for (int i = 0; i < 5; ++i)
    {
        endBatchWithTexture(_spriteBatchers[i], _textures[i], i);
    }
    
    OGL.enableBlending(true);
    OGL.bindFramebuffer(_framebuffers[1]);
    endBatchWithTexture(_spriteBatchers[5], _textures[5], 5);
    
    OGL.enableBlending(true);
    OGL.bindFramebuffer(_framebuffers[2]);
    for (int i = 6; i < 9; ++i)
    {
        endBatchWithTexture(_spriteBatchers[i], _textures[i], i);
    }
    
    int fbBegin = 0;
    int fbEnd = 3;
    
    OGL.enableBlending(false);
    OGL.bindFramebuffer(_framebuffers[9]);
    for (int i = fbBegin; i < fbEnd; ++i)
    {
        _screenRenderer.renderFramebuffer(_shaderManager.shader("shader_001"), _framebuffers[i]);
    }
    
    if (_engineState & SESS_DisplayTypes)
    {
        Texture& fontTexture = _textureManager.texture("texture_010");
        _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
    }
}

void StudioRenderer::renderEntities(std::vector<Entity*>& entities)
{
    for (Entity* e : entities)
    {
        TextureRegion& tr = ASSETS.findTextureRegion(e->getTextureMapping(), e->getStateTime());
        TextureDescriptor& td = ASSETS.findTextureDescriptor(e->getTextureMapping());
        
        _spriteBatchers[tr._layer].addSprite(tr, e->getPosition().x, e->getPosition().y, e->getWidth(), e->getHeight(), e->getAngle(), e->isFacingLeft());
        _textures[tr._layer] = td._name;
        
        if ((_engineState & SESS_DisplayTypes) &&
            _engineState & (1 << (tr._layer + SESS_LayerBitBegin)))
        {
            renderText(e->getEntityDef()._keyName.c_str(), e->getPosition().x, e->getPosition().y, FONT_ALIGN_CENTER);
        }
    }
}

void StudioRenderer::endBatchWithTexture(SpriteBatcher& sb, std::string textureName, int layer)
{
    if (textureName.empty())
    {
        return;
    }
    
    if (_engineState & (1 << (layer + SESS_LayerBitBegin)))
    {
        int c = clamp(layer, 0, 3);
        if (!(_engineState & SESS_DisplayParallax))
        {
            c = 3;
        }
        
        mat4_identity(_matrix);
        mat4_ortho(_matrix, _camBounds[c].left(), _camBounds[c].right(), _camBounds[c].bottom(), _camBounds[c].top(), -1, 1);
        
        sb.end(_shaderManager.shader("shader_002"), _matrix, _textureManager.texture(textureName));
    }
}

void StudioRenderer::renderActiveEntity()
{
    Entity* e = _input->_activeEntity;
    if (e == NULL)
    {
        e = _input->_lastActiveEntity;
    }
    
    if (e == NULL)
    {
        return;
    }
    
    mat4_identity(_matrix);
    mat4_ortho(_matrix, _camBounds[3].left(), _camBounds[3].right(), _camBounds[3].bottom(), _camBounds[3].top(), -1, 1);
    
    OGL.enableBlending(true);
    _activeEntitySpriteBatcher.begin();
    TextureRegion& tr = ASSETS.findTextureRegion(e->getTextureMapping(), e->getStateTime());
    TextureDescriptor& td = ASSETS.findTextureDescriptor(e->getTextureMapping());
    _activeEntitySpriteBatcher.addSprite(tr, e->getPosition().x, e->getPosition().y, e->getWidth(), e->getHeight(), e->getAngle(), e->isFacingLeft());
    _activeEntitySpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, _textureManager.texture(td._name), Color::DOUBLE);
}

void StudioRenderer::renderBox2D()
{
    if ((_engineState & SESS_DisplayBox2D) == 0)
    {
        return;
    }
    
    OGL.enableBlending(false);
    mat4_identity(_matrix);
    mat4_ortho(_matrix, _camBounds[3].left(), _camBounds[3].right(), _camBounds[3].bottom(), _camBounds[3].top(), -1, 1);
    _box2DDebugRenderer.render(&_shaderManager.shader("shader_003"), &_matrix, &_studioEngineState->_world.getWorld());
}

void StudioRenderer::renderGrid()
{
    if ((_engineState & SESS_DisplayGrid) == 0)
    {
        return;
    }
    
    float x = clamp(_camBounds[3].left(), 0, FLT_MAX);
    float y = clamp(_camBounds[3].bottom(), 0, FLT_MAX);
    float px = fmodf(x, CFG_MAIN._camWidth);
    float py = fmodf(y, CFG_MAIN._camHeight);
    float bx = x - px;
    float by = y - py;
    
    int left = x;
    int bottom = y;
    int leftAligned = bx;
    int bottomAligned = by;
    int camWidth = _camBounds[3].right() + 1;
    int camHeight = _camBounds[3].top() + 1;
    
    OGL.enableBlending(true);
    mat4_identity(_matrix);
    mat4_ortho(_matrix, _camBounds[3].left(), _camBounds[3].right(), _camBounds[3].bottom(), _camBounds[3].top(), -1, 1);
    
    _lineBatcher.begin();
    for (int i = left; i <= camWidth; ++i)
    {
        _lineBatcher.addLine(i, 0, i, camHeight);
    }
    for (int i = bottom; i <= camHeight; ++i)
    {
        _lineBatcher.addLine(0, i, camWidth, i);
    }
    static Color lineColor = Color::WHITE;
    lineColor._alpha = 0.25f;
    _lineBatcher.end(_shaderManager.shader("shader_003"), _matrix, lineColor);
    
    _lineBatcher.begin();
    for (int i = leftAligned; i <= camWidth; i += CFG_MAIN._camWidth)
    {
        _lineBatcher.addLine(i, 0, i, camHeight);
    }
    for (int i = bottomAligned; i <= camHeight; i += CFG_MAIN._camHeight)
    {
        _lineBatcher.addLine(0, i, camWidth, i);
    }
    _lineBatcher.end(_shaderManager.shader("shader_003"), _matrix, Color::GREEN);
    
    _lineBatcher.begin();
    _lineBatcher.addLine(0, 0, 0, camHeight);
    _lineBatcher.addLine(0, 0, camWidth, 0);
    _lineBatcher.end(_shaderManager.shader("shader_003"), _matrix, Color::RED);
}

void StudioRenderer::renderUI()
{
    OGL.enableBlending(false);
    
    mat4_identity(_matrix);
    mat4_ortho(_matrix, 0, CFG_MAIN._camWidth, 0, CFG_MAIN._camHeight, -1, 1);
    
    Texture& fontTexture = _textureManager.texture("texture_010");
    
    if (_engineState & SESS_DisplayLoadMapDialog)
    {
        /// Maps
        std::vector<MapDef>& maps = EntityLayoutMapper::getInstance().getMaps();
        int numMaps = static_cast<int>(maps.size());
        
        _fillPolygonBatcher.begin();
        int width = CFG_MAIN._camWidth / 3 + 4;
        int height = numMaps + 3;
        Rektangle window = Rektangle(CFG_MAIN._camWidth / 2 - width / 2, CFG_MAIN._camHeight - 4 - height - 1, width, height);
        Color windowColor = Color::BLUE;
        windowColor._alpha = 0.5f;
        _fillPolygonBatcher.addRektangle(window);
        _fillPolygonBatcher.end(_shaderManager.shader("shader_003"), _matrix, windowColor);
        
        int row = 2;
        static float padding = 1;
        
        _fontSpriteBatcher.begin();
        renderText("Load Map", CFG_MAIN._camWidth / 2, CFG_MAIN._camHeight - 4 - (row++ * padding), FONT_ALIGN_CENTER);
        _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
        
        ++row;
        
        for (int i = 0; i < numMaps; ++i)
        {
            MapDef& mp = maps[i];
            
            _fontSpriteBatcher.begin();
            renderText(StringUtil::format("%s | %s", mp._name.c_str(), mp._value.c_str()).c_str(), CFG_MAIN._camWidth / 2, CFG_MAIN._camHeight - 4 - (row++ * padding), FONT_ALIGN_CENTER);
            _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture, i == _input->_selectionIndex ? Color::WHITE : Color::BLACK);
        }
    }
    else if (_engineState & SESS_DisplayEntities)
    {
        const std::vector<EntityDef*>& entityDescriptors = ENTITY_MAPPER.getEntityDescriptors();
        int numEntities = static_cast<int>(entityDescriptors.size());
        
        int width = CFG_MAIN._camWidth * 0.7f;
        int height = numEntities + 3;
        
        mat4_identity(_matrix);
        mat4_ortho(_matrix, 0, CFG_MAIN._camWidth, 0, CFG_MAIN._camHeight, -1, 1);
        
        _fillPolygonBatcher.begin();
        
        Rektangle window = Rektangle(1, CFG_MAIN._camHeight - height - 1, width, height);
        Color windowColor = Color::BLUE;
        windowColor._alpha = 0.5f;
        _fillPolygonBatcher.addRektangle(window);
        _fillPolygonBatcher.end(_shaderManager.shader("shader_003"), _matrix, windowColor);
        
        int selectionIndex = _input->_selectionIndex;
        int row = 1;
        static float padding = 5;
        
        _fontSpriteBatcher.begin();
        renderText("Entities", width / 2, CFG_MAIN._camHeight - 1 - (row * padding), FONT_ALIGN_CENTER);
        _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
        
        ++row;
        
        for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
        {
            _spriteBatchers[i].begin();
        }
        
        std::fill(_textures, _textures + NUM_SPRITE_BATCHERS, "");
        
        for (int i = clamp(selectionIndex - 2, 0, numEntities - 1); i < numEntities; ++i)
        {
            EntityDef* ed = entityDescriptors[i];
            
            _fontSpriteBatcher.begin();
            renderText(StringUtil::format("%s | %s", ed->_keyName.c_str(), ed->_name.c_str()).c_str(), 7, CFG_MAIN._camHeight - 1 - (row * padding), FONT_ALIGN_LEFT);
            _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture, i == selectionIndex ? Color::WHITE : Color::BLACK);
            
            TextureRegion& tr = ASSETS.findTextureRegion(ed->_textureMappings[0], 0);
            TextureDescriptor& td = ASSETS.findTextureDescriptor(ed->_textureMappings[0]);
            _spriteBatchers[tr._layer].addSprite(tr, 4, CFG_MAIN._camHeight - 1 - (row * padding), 4, 4, 0);
            _textures[tr._layer] = td._name;
            
            ++row;
        }
        
        for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
        {
            if (_textures[i].length() > 0)
            {
                _spriteBatchers[i].end(_shaderManager.shader("shader_002"), _matrix, _textureManager.texture(_textures[i]));
            }
        }
    }
    else if (_engineState & SESS_DisplayControls)
    {
        /// Controls
        _fillPolygonBatcher.begin();
        int width = 22;
        int height = 18;
        Rektangle window = Rektangle(CFG_MAIN._camWidth - width - 1, CFG_MAIN._camHeight - height - 3, width, height);
        Color windowColor = Color::BLUE;
        windowColor._alpha = 0.5f;
        _fillPolygonBatcher.addRektangle(window);
        _fillPolygonBatcher.end(_shaderManager.shader("shader_003"), _matrix, windowColor);
        
        _fontSpriteBatcher.begin();
        
        int row = 4;
        static float padding = 1;
        
        renderText(StringUtil::format("[B]   Box2D Debug %s", _engineState & SESS_DisplayBox2D ? " ON" : "OFF").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[G]          Grid %s", _engineState & SESS_DisplayGrid ? " ON" : "OFF").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[D]   Debug Types %s", _engineState & SESS_DisplayTypes ? " ON" : "OFF").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[P]      Parallax %s", _engineState & SESS_DisplayParallax ? " ON" : "OFF").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        ++row;
        renderText(StringUtil::format("[R]      Reset Camera").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[X]       Reset World").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[T]         Test Zone").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        ++row;
        renderText(StringUtil::format("[N]               New").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[L]              Load").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[S]              Save").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[CTRL+S]      Save As").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        ++row;
        renderText(StringUtil::format("[C]  %s Controls", _engineState & SESS_DisplayControls ? "Hide   " : "Display").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[A]  %s   Assets", _engineState & SESS_DisplayAssets ? "Hide   " : "Display").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        renderText(StringUtil::format("[E]  %s Entities", _engineState & SESS_DisplayEntities ? "Hide   " : "Display").c_str(), CFG_MAIN._camWidth - 2, CFG_MAIN._camHeight - (row++ * padding), FONT_ALIGN_RIGHT);
        
        _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
    }
    
    {
        /// Top Bar
        _fillPolygonBatcher.begin();
        Rektangle bar = Rektangle(0, CFG_MAIN._camHeight - 2, CFG_MAIN._camWidth, 2);
        Color barColor = Color(0.33f, 0.33f, 0.33f, 0.85f);
        _fillPolygonBatcher.addRektangle(bar);
        _fillPolygonBatcher.end(_shaderManager.shader("shader_003"), _matrix, barColor);
        
        int column = 1;
        static float padding = 1;
        static float textY = CFG_MAIN._camHeight - 2 + 1.5f;
        static float textY2 = CFG_MAIN._camHeight - 2 + 0.5f;
        
        for (int i = 0; i < SESS_NumLayers; ++i)
        {
            _fontSpriteBatcher.begin();
            renderText(StringUtil::format("%d", i).c_str(), 1 + (column++ * padding), textY, FONT_ALIGN_RIGHT);
            _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture, _engineState & (1 << (i + SESS_LayerBitBegin)) ? Color::WHITE : Color::BLACK);
        }
        
        _fontSpriteBatcher.begin();
        renderText("Layers", 1 + column / 2.0f, textY2, FONT_ALIGN_CENTER);
        _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
        
        {
            /// Render Map Info in the center of the bar
            _fontSpriteBatcher.begin();
            renderText(StringUtil::format("%s | %s", _studioEngineState->_world.getMapName().c_str(), _studioEngineState->_world.getMapFileName().c_str()).c_str(), CFG_MAIN._camWidth / 2, textY, FONT_ALIGN_CENTER);
            _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
            
            _fontSpriteBatcher.begin();
            renderText("Map", CFG_MAIN._camWidth / 2, textY2, FONT_ALIGN_CENTER);
            _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
        }
        
        column = 56;
        {
            _fontSpriteBatcher.begin();
            renderText("C", 1 + (column++ * padding), textY, FONT_ALIGN_LEFT);
            _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture, _engineState & SESS_DisplayControls ? Color::WHITE : Color::BLACK);
            
            _fontSpriteBatcher.begin();
            renderText("A", 1 + (column++ * padding), textY, FONT_ALIGN_LEFT);
            _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture, _engineState & SESS_DisplayAssets ? Color::WHITE : Color::BLACK);
            
            _fontSpriteBatcher.begin();
            renderText("E", 1 + (column++ * padding), textY, FONT_ALIGN_LEFT);
            _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture, _engineState & SESS_DisplayEntities ? Color::WHITE : Color::BLACK);
            
            _fontSpriteBatcher.begin();
            renderText("Windows", 58, textY2, FONT_ALIGN_CENTER);
            _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
        }
    }
    
    {
        /// Toasts
        int y = CFG_MAIN._camHeight - 4;
        _fillPolygonBatcher.begin();
        _fontSpriteBatcher.begin();
        for (std::string t : _toasts)
        {
            Rektangle window = Rektangle(CFG_MAIN._camWidth / 2 - t.length() / 2.0f - 1, y - 1, t.length() + 1, 2);
            _fillPolygonBatcher.addRektangle(window);
            renderText(t.c_str(), CFG_MAIN._camWidth / 2, y, FONT_ALIGN_CENTER);
            y -= 2;
        }
        Color windowColor = Color::BLUE;
        windowColor._alpha = 0.5f;
        _fillPolygonBatcher.end(_shaderManager.shader("shader_003"), _matrix, windowColor);
        _fontSpriteBatcher.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
    }
}

void StudioRenderer::renderText(const char* inStr, float x, float y, int justification)
{
    float fgWidth = CFG_MAIN._camWidth / 64.0f;
    float fgHeight = fgWidth * (75.0f / 64.0f);

    std::string text(inStr);

    _font.renderText(_fontSpriteBatcher, text, x, y, fgWidth, fgHeight, justification);
}
