//
//  GameRenderer.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 2/22/14.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "GameRenderer.hpp"

#include "Entity.hpp"
#include "GameEngineState.hpp"

#include "TextureManager.hpp"
#include "Font.hpp"
#include "World.hpp"
#include "TextureRegion.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
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
#include "PlayerController.hpp"
#include "NetworkManagerClient.hpp"
#include "WeightedTimedMovingAverage.hpp"
#include "Server.hpp"
#include "GowAudioEngine.hpp"

#include <sstream>
#include <assert.h>
#include <cfloat>

GameRenderer::GameRenderer() :
_textureManager(),
_shaderManager(),
_spriteBatchers{
    SpriteBatcher(256),
    SpriteBatcher(256),
    SpriteBatcher(256),
    SpriteBatcher(256),
    SpriteBatcher(256),
    SpriteBatcher(256),
    SpriteBatcher(256),
    SpriteBatcher(256),
    SpriteBatcher(256),
    SpriteBatcher(256)
},
_staticFontSpriteBatcher(512),
_dynamicFontSpriteBatcher(512),
_box2DDebugRenderer(2048),
_lightRenderer(),
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
    Rektangle(),
    Rektangle(),
    Rektangle(),
    Rektangle()
},
_gameEngineState(NULL),
_map(0),
_engineState(0)
{
    // Empty
}

GameRenderer::~GameRenderer()
{
    // Empty
}

void GameRenderer::createDeviceDependentResources()
{
    _gameEngineState = GameEngineState::getInstance();
    
    _textureManager.createDeviceDependentResources();
    _shaderManager.createDeviceDependentResources();
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        _spriteBatchers[i].createDeviceDependentResources();
    }
    
    _staticFontSpriteBatcher.createDeviceDependentResources();
    _dynamicFontSpriteBatcher.createDeviceDependentResources();
    _box2DDebugRenderer.createDeviceDependentResources();
    _lightRenderer.createDeviceDependentResources();
    _screenRenderer.createDeviceDependentResources();
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        OGL.loadFramebuffer(_framebuffers[i], GL_LINEAR, GL_LINEAR);
    }
}

void GameRenderer::onWindowSizeChanged(int screenWidth, int screenHeight)
{
    _screenRenderer.onWindowSizeChanged(screenWidth, screenHeight);
}

void GameRenderer::releaseDeviceDependentResources()
{
    _textureManager.releaseDeviceDependentResources();
    _shaderManager.releaseDeviceDependentResources();
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        _spriteBatchers[i].releaseDeviceDependentResources();
    }
    
    _staticFontSpriteBatcher.releaseDeviceDependentResources();
    _dynamicFontSpriteBatcher.releaseDeviceDependentResources();
    _box2DDebugRenderer.releaseDeviceDependentResources();
    _lightRenderer.releaseDeviceDependentResources();
    _screenRenderer.releaseDeviceDependentResources();
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        OGL.unloadFramebuffer(_framebuffers[i]);
    }
}

void GameRenderer::render()
{
    _engineState = _gameEngineState->_state;
    
    updateCamera();
    renderWorld();
    renderBox2D();
    renderUI();
    
    _screenRenderer.renderToScreen(_shaderManager.shader("shader_001"), _framebuffers[9]);
}

void GameRenderer::updateCamera()
{
    // Adjust camera based on the player position
    float x = 0;
    float y = 0;
    
    _playerLights.clear();
    _lights.clear();
    
    bool isCamInitialized = false;
    
    for (Entity* e : _gameEngineState->_world->getPlayers())
    {
        float pX = e->getPosition().x;
        float pY = e->getPosition().y;
        float lightPosY = pY - e->getHeight() / 2 + e->getHeight() * CFG_MAIN._robotLightPositionFactorY;
        
        _playerLights.push_back(LightDef(pX, lightPosY, CFG_MAIN._playerLightColor[0], CFG_MAIN._playerLightColor[1], CFG_MAIN._playerLightColor[2], CFG_MAIN._playerLightColor[3]));
        
        PlayerNetworkController* p = static_cast<PlayerNetworkController*>(e->getNetworkController());
        if (!isCamInitialized && p->isLocalPlayer())
        {
            x = pX;
            y = pY;
            
            x -= CFG_MAIN._camWidth * 0.5f;
            y -= CFG_MAIN._camHeight * 0.5f;
            
            x = clamp(x, 0, FLT_MAX);
            y = clamp(y, 0, FLT_MAX);
            
            isCamInitialized = true;
        }
    }
    
    /// Temporary
    _lights.push_back(LightDef(CFG_MAIN._tempStaticLight1[0], CFG_MAIN._tempStaticLight1[1], CFG_MAIN._tempStaticLight1[2], CFG_MAIN._tempStaticLight1[3], CFG_MAIN._tempStaticLight1[4], CFG_MAIN._tempStaticLight1[5]));
    _lights.push_back(LightDef(CFG_MAIN._tempStaticLight2[0], CFG_MAIN._tempStaticLight2[1], CFG_MAIN._tempStaticLight2[2], CFG_MAIN._tempStaticLight2[3], CFG_MAIN._tempStaticLight2[4], CFG_MAIN._tempStaticLight2[5]));
    
    _camBounds[3]._lowerLeft.set(x, y);
    _camBounds[2]._lowerLeft.set(x * CFG_MAIN._parallaxLayer2FactorX, y * CFG_MAIN._parallaxLayer2FactorY);
    _camBounds[1]._lowerLeft.set(x * CFG_MAIN._parallaxLayer1FactorX, y * CFG_MAIN._parallaxLayer1FactorY);
    _camBounds[0]._lowerLeft.set(x * CFG_MAIN._parallaxLayer0FactorX, y * CFG_MAIN._parallaxLayer0FactorY);
    
    for (int i = 0; i < NUM_CAMERAS; ++i)
    {
        _camBounds[i]._width = CFG_MAIN._camWidth;
        _camBounds[i]._height = CFG_MAIN._camHeight;
    }
}

void GameRenderer::renderWorld()
{
    World* world = _gameEngineState->_world;
    
    for (int i = 0; i < NUM_SPRITE_BATCHERS; ++i)
    {
        _spriteBatchers[i].begin();
    }
    
    std::fill(_textures, _textures + NUM_SPRITE_BATCHERS, "");
    
    renderEntities(world->getLayers());
    renderEntities(world->getStaticEntities());
    renderEntities(world->getDynamicEntities());
    
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
    
    if (_engineState & GESS_Lighting)
    {
        fbBegin = 6;
        fbEnd = 9;
        
        renderLighting();
    }
    
    OGL.enableBlending(false);
    OGL.bindFramebuffer(_framebuffers[9]);
    for (int i = fbBegin; i < fbEnd; ++i)
    {
        _screenRenderer.renderFramebuffer(_shaderManager.shader("shader_001"), _framebuffers[i]);
    }
}

void GameRenderer::renderEntities(std::vector<Entity*>& entities)
{
    for (Entity* e : entities)
    {
        TextureRegion tr = ASSETS.findTextureRegion(e->getTextureMapping(), e->getStateTime());
        TextureDescriptor& td = ASSETS.findTextureDescriptor(e->getTextureMapping());
        
        int layer = tr._layer;
        _textures[layer] = td._name;
        _normals[layer] = td._normalMapName;
        
        _spriteBatchers[layer].addSprite(tr, e->getPosition().x, e->getPosition().y, e->getWidth(), e->getHeight(), e->getAngle(), e->isFacingLeft());
    }
}

void GameRenderer::renderLighting()
{
    OGL.enableBlending(true);
    OGL.bindFramebuffer(_framebuffers[3]);
    for (int i = 0; i < 5; ++i)
    {
        endBatchWithTexture(_spriteBatchers[i], _normals[i], i);
    }
    
    OGL.enableBlending(true);
    OGL.bindFramebuffer(_framebuffers[4]);
    endBatchWithTexture(_spriteBatchers[5], _normals[5], 5);
    
    OGL.enableBlending(true);
    OGL.bindFramebuffer(_framebuffers[5]);
    for (int i = 6; i < 9; ++i)
    {
        endBatchWithTexture(_spriteBatchers[i], _normals[i], i);
    }
    
    OGL.enableBlending(false);
    
    {
        _lightRenderer.resetLights();
        _lightRenderer.configAmbientLight(CFG_MAIN._ambientColor[0], CFG_MAIN._ambientColor[1], CFG_MAIN._ambientColor[2], CFG_MAIN._ambientColor[3]);
        _lightRenderer.configureFallOff(CFG_MAIN._fallOff[0], CFG_MAIN._fallOff[1], CFG_MAIN._fallOff[2]);
        for (int j = 0 ; j < _playerLights.size(); ++j)
        {
            LightDef& ld = _playerLights[j];
            _lightRenderer.addLight(ld._lightPosX, ld._lightPosY, CFG_MAIN._behindPlayerLightZFactor * CFG_MAIN._playerLightZ, ld._lightColorR, ld._lightColorG, ld._lightColorB, ld._lightColorA);
        }
        for (int j = 0 ; j < _lights.size(); ++j)
        {
            LightDef& ld = _lights[j];
            _lightRenderer.addLight(ld._lightPosX, ld._lightPosY, CFG_MAIN._behindPlayerLightZFactor * CFG_MAIN._playerLightZ, ld._lightColorR, ld._lightColorG, ld._lightColorB, ld._lightColorA);
        }
        
        OGL.bindFramebuffer(_framebuffers[6]);
        _lightRenderer.render(_shaderManager.shader("shader_004"), _matrix, _framebuffers[0]._texture, _framebuffers[3]._texture);
    }
    
    {
        _lightRenderer.resetLights();
        _lightRenderer.configAmbientLight(1, 1, 1, 1);
        _lightRenderer.configureFallOff(CFG_MAIN._fallOff[0], CFG_MAIN._fallOff[1], CFG_MAIN._fallOff[2]);
        for (int j = 0 ; j < _lights.size(); ++j)
        {
            LightDef& ld = _lights[j];
            _lightRenderer.addLight(ld._lightPosX, ld._lightPosY, CFG_MAIN._playerLightZ, ld._lightColorR, ld._lightColorG, ld._lightColorB, ld._lightColorA);
        }
        
        OGL.bindFramebuffer(_framebuffers[7]);
        _lightRenderer.render(_shaderManager.shader("shader_004"), _matrix, _framebuffers[1]._texture, _framebuffers[4]._texture);
    }
    
    {
        _lightRenderer.resetLights();
        _lightRenderer.configAmbientLight(CFG_MAIN._ambientColor[0], CFG_MAIN._ambientColor[1], CFG_MAIN._ambientColor[2], CFG_MAIN._ambientColor[3]);
        _lightRenderer.configureFallOff(CFG_MAIN._fallOff[0], CFG_MAIN._fallOff[1], CFG_MAIN._fallOff[2]);
        for (int j = 0 ; j < _playerLights.size(); ++j)
        {
            LightDef& ld = _playerLights[j];
            _lightRenderer.addLight(ld._lightPosX, ld._lightPosY, CFG_MAIN._frontPlayerLightZFactor * CFG_MAIN._playerLightZ, ld._lightColorR, ld._lightColorG, ld._lightColorB, ld._lightColorA);
        }
        for (int j = 0 ; j < _lights.size(); ++j)
        {
            LightDef& ld = _lights[j];
            _lightRenderer.addLight(ld._lightPosX, ld._lightPosY, CFG_MAIN._frontPlayerLightZFactor * CFG_MAIN._playerLightZ, ld._lightColorR, ld._lightColorG, ld._lightColorB, ld._lightColorA);
        }
        
        OGL.bindFramebuffer(_framebuffers[8]);
        _lightRenderer.render(_shaderManager.shader("shader_004"), _matrix, _framebuffers[2]._texture, _framebuffers[5]._texture);
    }
}

void GameRenderer::endBatchWithTexture(SpriteBatcher& sb, std::string textureName, int layer)
{
    if (textureName.empty())
    {
        return;
    }
    
    int c = clamp(layer, 0, 3);
    
    mat4_identity(_matrix);
    mat4_ortho(_matrix, _camBounds[c].left(), _camBounds[c].right(), _camBounds[c].bottom(), _camBounds[c].top(), -1, 1);
    
    sb.end(_shaderManager.shader("shader_002"), _matrix, _textureManager.texture(textureName));
}

void GameRenderer::renderBox2D()
{
    if ((_engineState & GESS_DisplayBox2D) == 0)
    {
        return;
    }
    
    mat4_identity(_matrix);
    mat4_ortho(_matrix, _camBounds[3].left(), _camBounds[3].right(), _camBounds[3].bottom(), _camBounds[3].top(), -1, 1);
    _box2DDebugRenderer.render(&_shaderManager.shader("shader_003"), &_matrix, &_gameEngineState->_world->getWorld());
}

void GameRenderer::renderUI()
{
    if ((_engineState & GESS_DisplayUI) == 0)
    {
        return;
    }
    
    mat4_identity(_matrix);
    mat4_ortho(_matrix, 0, CFG_MAIN._camWidth, 0, CFG_MAIN._camHeight, -1, 1);
    
    int jl = FONT_ALIGN_LEFT;
    int jr = FONT_ALIGN_RIGHT;
    float fontX = CFG_MAIN._camWidth - 0.5f;
    float camHeight = CFG_MAIN._camHeight;
    SpriteBatcher& dSb = _dynamicFontSpriteBatcher;
    SpriteBatcher& sSb = _staticFontSpriteBatcher;
    
    Texture& fontTexture = _textureManager.texture("texture_010");
    
    sSb.begin();
    
    if (NW_MGR_CLIENT && NW_MGR_CLIENT->state() == NCS_Welcomed)
    {
        dSb.begin();
        
        int row = 1;
        static float padding = 1;
        
        int fps = FPS_UTIL.getFPS();
        renderText(dSb, StringUtil::format("FPS %d", fps).c_str(), fontX, camHeight - (row++ * padding), jr);
        
        float rttMS = NW_MGR_CLIENT->getAvgRoundTripTime().getValue() * 1000.f;
        renderText(dSb, StringUtil::format("RTT %d ms", static_cast<int>(rttMS)).c_str(), fontX, camHeight - (row++ * padding), jr);
        
        const WeightedTimedMovingAverage& bpsIn = NW_MGR_CLIENT->getBytesReceivedPerSecond();
        int bpsInInt = static_cast<int>(bpsIn.getValue());
        renderText(dSb, StringUtil::format(" In %d Bps", bpsInInt).c_str(), fontX, camHeight - (row++ * padding), jr);
        
        const WeightedTimedMovingAverage& bpsOut = NW_MGR_CLIENT->getBytesSentPerSecond();
        int bpsOutInt = static_cast<int>(bpsOut.getValue());
        renderText(dSb, StringUtil::format("Out %d Bps", bpsOutInt).c_str(), fontX, camHeight - (row++ * padding), jr);
        
        dSb.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
        
        // Controls
        ++row;
        
        renderText(sSb, StringUtil::format("[N]         Sound %s", GOW_AUDIO->areSoundsDisabled() ? " OFF" : "  ON").c_str(), fontX, camHeight - (row++ * padding), jr);
        renderText(sSb, StringUtil::format("[M]         Music %s", GOW_AUDIO->isMusicDisabled() ? " OFF" : "  ON").c_str(), fontX, camHeight - (row++ * padding), jr);
        renderText(sSb, StringUtil::format("[B]   Box2D Debug %s", _engineState & GESS_DisplayBox2D ? "  ON" : " OFF").c_str(), fontX, camHeight - (row++ * padding), jr);
        renderText(sSb, StringUtil::format("[I] Interpolation %s", _engineState & GESS_Interpolation ? "  ON" : " OFF").c_str(), fontX, camHeight - (row++ * padding), jr);
        std::string lightZ = StringUtil::format("%f", CFG_MAIN._playerLightZ);
        renderText(sSb, StringUtil::format("[L]  Lighting %s", _engineState & GESS_Lighting ? lightZ.c_str() : "     OFF").c_str(), fontX, camHeight - (row++ * padding), jr);
        renderText(sSb, StringUtil::format("[U]    Display UI %s", _engineState & GESS_DisplayUI ? "  ON" : " OFF").c_str(), fontX, camHeight - (row++ * padding), jr);
        
        Server* server = Server::getInstance();
        if (server && !(server->getFlags() & ServerFlag_TestSession))
        {
            renderText(sSb, StringUtil::format("[T]    Toggle Map %s", _gameEngineState->_world->getMapName().c_str()).c_str(), fontX, camHeight - (row++ * padding), jr);
        }
        else
        {
            renderText(sSb, StringUtil::format("              Map %s", _gameEngineState->_world->getMapName().c_str()).c_str(), fontX, camHeight - (row++ * padding), jr);
        }
        
        bool activePlayerIds[4] = {false};
        
        std::vector<Entity*> players = _gameEngineState->_world->getPlayers();
        for (Entity* entity : players)
        {
            PlayerController* robot = static_cast<PlayerController*>(entity->getController());
            
            int playerID = robot->getPlayerID();
            if (playerID >= 1 && playerID <= 4)
            {
                renderText(sSb, StringUtil::format("%i|%s", playerID, robot->getPlayerName().c_str()).c_str(), 0.5f, camHeight - (playerID * 1.0f), jl);
                activePlayerIds[playerID - 1] = true;
            }
        }
        
        for (int i = 0; i < MAX_NUM_PLAYERS_PER_SERVER; ++i)
        {
            if (!activePlayerIds[i])
            {
                renderText(sSb, StringUtil::format("%i|%s", (i + 1), "Connect a controller to join...").c_str(), 0.5f, camHeight - ((i + 1) * 1.0f), jl);
            }
        }
    }
    else
    {
        renderText(sSb, StringUtil::format("%s, [ESC] to exit", "Joining Server...").c_str(), 0.5f, camHeight - 4, jl);
    }
    
    sSb.end(_shaderManager.shader("shader_002"), _matrix, fontTexture);
}

void GameRenderer::renderText(SpriteBatcher& sb, const char* inStr, float x, float y, int justification)
{
    float fgWidth = CFG_MAIN._camWidth / 64;
    float fgHeight = fgWidth * 1.171875f;
    
    std::string text(inStr);
    
    _font.renderText(sb, text, x, y, fgWidth, fgHeight, justification);
}
