//
//  StudioInputManager.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/4/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "StudioInputManager.hpp"

#include "EntityIDManager.hpp"
#include "StudioEngineState.hpp"
#include "Entity.hpp"

#include "InputManager.hpp"
#include "Constants.hpp"
#include "KeyboardLookup.hpp"
#include "StringUtil.hpp"
#include "MathUtil.hpp"
#include "StudioRenderer.hpp"
#include "World.hpp"
#include "EntityLayoutMapper.hpp"
#include "EntityMapper.hpp"
#include "OverlapTester.hpp"
#include "Rektangle.hpp"
#include "TextureRegion.hpp"
#include "Assets.hpp"
#include "Macros.hpp"
#include "InstanceManager.hpp"
#include "MainConfig.hpp"
#include "CursorConverter.hpp"

#include <sstream>

StudioInputManager* StudioInputManager::s_instance = NULL;
uint32_t StudioInputManager::s_testMap = 'TEST';

void StudioInputManager::create()
{
    assert(!s_instance);
    
    s_instance = new StudioInputManager();
}

StudioInputManager * StudioInputManager::getInstance()
{
    return s_instance;
}

void StudioInputManager::destroy()
{
    assert(s_instance);
    
    delete s_instance;
    s_instance = NULL;
}

int StudioInputManager::getMenuState()
{
    return _inputState;
}

void StudioInputManager::update()
{
    _inputState = SIMS_NONE;
    
    uint32_t state = _studioEngineState->_state;
    if (state & SESS_DisplayLoadMapDialog)
    {
        handleLoadMapDialogInput();
    }
    else if (state & SESS_DisplayEntities)
    {
        handleEntitiesInput();
    }
    else
    {
        handleDefaultInput();
        updateCamera();
    }
    
    int w = CFG_MAIN._camWidth * _scrollValue;
    int h = CFG_MAIN._camHeight * _scrollValue;
    _studioEngineState->_renderer.update(_cursor.x(), _cursor.y(), w, h, _scrollValue);
}

void StudioInputManager::handleDefaultInput()
{
    std::vector<CursorEvent*>& cursorEvents = INPUT_MGR.getCursorEvents();
    for (std::vector<CursorEvent *>::iterator i = cursorEvents.begin(); i != cursorEvents.end(); ++i)
    {
        CursorEvent& e = *(*i);
        switch (e._type)
        {
            case CursorEventType_DOWN:
            {
                _hasTouchedScreen = true;
                
                Vector2& c = CURSOR_CONVERTER.convert(e);
                _downCursor.set(c);
                _dragCursor.set(c);
                _deltaCursor.set(0, 0);
                
                _activeEntityDeltaCursor.set(0, 0);
                _activeEntityCursor.set(_cursor);
                _activeEntityCursor += _downCursor;
                
                if (e._isAlt)
                {
                    if (_lastActiveEntity)
                    {
                        onEntityRemoved(_lastActiveEntity);
                        _studioEngineState->_world.removeEntity(_lastActiveEntity);
                        
                        _activeEntity = NULL;
                        _lastActiveEntity = NULL;
                    }
                }
                else
                {
                    _activeEntity = getEntityAtPosition(_activeEntityCursor.x(), _activeEntityCursor.y());
                    if (_activeEntity != NULL)
                    {
                        _activeEntityDeltaCursor.set(_activeEntity->getPosition().x, _activeEntity->getPosition().y);
                        _activeEntityDeltaCursor -= _activeEntityCursor;
                        
                        _lastActiveEntity = NULL;
                    }
                }
                
                continue;
            }
            case CursorEventType_DRAGGED:
            {
                Vector2& c = CURSOR_CONVERTER.convert(e);
                Vector2 delta = c;
                delta -= _dragCursor;
                _dragCursor.set(c);
                _deltaCursor.set(delta);
                
                if (_activeEntity != NULL)
                {
                    _activeEntityCursor += _deltaCursor;
                    Vector2 entityPos = _activeEntityCursor;
                    entityPos += _activeEntityDeltaCursor;
                    _activeEntity->setPosition(b2Vec2(entityPos.x(), entityPos.y()));
                }
                
                continue;
            }
            case CursorEventType_UP:
            {
                Vector2& c = CURSOR_CONVERTER.convert(e);
                _upCursor.set(c);
                _deltaCursor.set(0, 0);
                
                if (_activeEntity != NULL)
                {
                    const b2Vec2& position = _activeEntity->getPosition();
                    float width = _activeEntity->getWidth();
                    float height = _activeEntity->getHeight();
                    float x = clamp(position.x, width / 2, FLT_MAX);
                    float y = clamp(position.y, height / 2, FLT_MAX);
                    x = floor(x);
                    y = floor(y);
                    _activeEntity->setPosition(b2Vec2(x, y));
                    
                    _lastActiveEntity = _activeEntity;
                    
                    _activeEntity = NULL;
                }
                
                break;
            }
            case CursorEventType_MOVED:
            {
                Vector2& c = CURSOR_CONVERTER.convert(e);
                _cursorPosition.set(c);
                
                continue;
            }
            case CursorEventType_SCROLL:
            {
                _rawScrollValue = clamp(_rawScrollValue + -e._y, 1, 16);
                _scrollValue = clamp(_rawScrollValue, 1, 16);
                CURSOR_CONVERTER.setMatrixSize(CFG_MAIN._camWidth * _scrollValue, CFG_MAIN._camHeight * _scrollValue);
                
                continue;
            }
            default:
                break;
        }
    }
    
    std::vector<KeyboardEvent*>& keyboardEvents = INPUT_MGR.getKeyboardEvents();
    for (std::vector<KeyboardEvent *>::iterator i = keyboardEvents.begin(); i != keyboardEvents.end(); ++i)
    {
        KeyboardEvent& e = *(*i);
        switch (e._key)
        {
            case GOW_KEY_CMD:
            case GOW_KEY_CTRL:
                _isControl = e.isPressed();
                continue;
            case GOW_KEY_ZERO:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer0 : 0;
                continue;
            case GOW_KEY_ONE:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer1 : 0;
                continue;
            case GOW_KEY_TWO:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer2 : 0;
                continue;
            case GOW_KEY_THREE:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer3 : 0;
                continue;
            case GOW_KEY_FOUR:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer4 : 0;
                continue;
            case GOW_KEY_FIVE:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer5 : 0;
                continue;
            case GOW_KEY_SIX:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer6 : 0;
                continue;
            case GOW_KEY_SEVEN:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer7 : 0;
                continue;
            case GOW_KEY_EIGHT:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer8 : 0;
                continue;
            case GOW_KEY_NINE:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer9 : 0;
                continue;
            case GOW_KEY_D:
                _studioEngineState->_state ^= e.isDown() ? SESS_DisplayTypes : 0;
                continue;
            case GOW_KEY_ARROW_LEFT:
                _isPanningLeft = e.isPressed();
                continue;
            case GOW_KEY_ARROW_RIGHT:
                _isPanningRight = e.isPressed();
                continue;
            case GOW_KEY_ARROW_DOWN:
                _isPanningDown = e.isPressed();
                continue;
            case GOW_KEY_ARROW_UP:
                _isPanningUp = e.isPressed();
                continue;
            case GOW_KEY_C:
                _studioEngineState->_state ^= e.isDown() ? SESS_DisplayControls : 0;
                continue;
            case GOW_KEY_A:
            {
                _studioEngineState->_state ^= e.isDown() ? SESS_DisplayAssets : 0;
                if (e.isDown())
                {
                    _studioEngineState->_renderer.displayToast("Assets Management not yet implemented...");
                }
                continue;
            }
            case GOW_KEY_E:
            {
                if (e.isDown())
                {
                    if (_studioEngineState->_world.isMapLoaded())
                    {
                        _studioEngineState->_state |= SESS_DisplayEntities;
                    }
                    else
                    {
                        _studioEngineState->_renderer.displayToast("Load a Map first!");
                    }
                }
                continue;
            }
            case GOW_KEY_R:
            {
                if (e.isDown())
                {
                    resetCamera();
                }
                continue;
            }
            case GOW_KEY_N:
            {
                _studioEngineState->_state |= e.isDown() ? SESS_DisplayNewMapDialog : 0;
                if (e.isDown())
                {
                    _studioEngineState->_renderer.displayToast("New Map not yet implemented...");
                }
                continue;
            }
            case GOW_KEY_L:
                _studioEngineState->_state |= e.isDown() ? SESS_DisplayLoadMapDialog : 0;
                continue;
            case GOW_KEY_X:
            {
                if (e.isDown())
                {
                    _studioEngineState->_world.clear();
                    onMapLoaded();
                }
                continue;
            }
            case GOW_KEY_T:
            {
                if (e.isDown())
                {
                    if (_studioEngineState->_world.getDynamicEntities().size() > MAX_NUM_DYNAMIC_ENTITIES)
                    {
                        _studioEngineState->_renderer.displayToast(StringUtil::format("Cannot have more than %d dynamic entities!", MAX_NUM_DYNAMIC_ENTITIES));
                    }
                    else
                    {
                        _studioEngineState->_world.saveMapAs(s_testMap);
                        _studioEngineState->_testFunc(_studioEngineState->_engine, s_testMap);
                        return;
                    }
                }
                continue;
            }
            case GOW_KEY_S:
            {
                if (e.isDown())
                {
                    if (!_studioEngineState->_world.isMapLoaded())
                    {
                        _studioEngineState->_renderer.displayToast("Load a Map first!");
                        return;
                    }
                    
                    if (_studioEngineState->_world.getDynamicEntities().size() > MAX_NUM_DYNAMIC_ENTITIES)
                    {
                        _studioEngineState->_renderer.displayToast(StringUtil::format("Cannot have more than %d dynamic entities!", MAX_NUM_DYNAMIC_ENTITIES));
                    }
                    else
                    {
                        if (_isControl)
                        {
                            _studioEngineState->_state |= e.isDown() ? SESS_DisplaySaveMapAsDialog : 0;
                            _studioEngineState->_renderer.displayToast("Save As not yet implemented...");
                        }
                        else
                        {
                            _studioEngineState->_world.saveMap();
                            _studioEngineState->_renderer.displayToast(StringUtil::format("%s saved!", _studioEngineState->_world.getMapName().c_str()).c_str());
                        }
                    }
                }
                continue;
            }
            case GOW_KEY_P:
                _studioEngineState->_state ^= e.isDown() ? SESS_DisplayParallax : 0;
                continue;
            case GOW_KEY_B:
                _studioEngineState->_state ^= e.isDown() ? SESS_DisplayBox2D : 0;
                continue;
            case GOW_KEY_G:
                _studioEngineState->_state ^= e.isDown() ? SESS_DisplayGrid : 0;
                continue;
            case GOW_KEY_ESCAPE:
                _inputState = e.isDown() ? SIMS_ESCAPE : SIMS_NONE;
                continue;
            default:
                continue;
        }
    }
}

void StudioInputManager::handleEntitiesInput()
{
    std::vector<KeyboardEvent*>& keyboardEvents = INPUT_MGR.getKeyboardEvents();
    for (std::vector<KeyboardEvent *>::iterator i = keyboardEvents.begin(); i != keyboardEvents.end(); ++i)
    {
        KeyboardEvent& e = *(*i);
        switch (e._key)
        {
            case GOW_KEY_ZERO:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer0 : 0;
                continue;
            case GOW_KEY_ONE:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer1 : 0;
                continue;
            case GOW_KEY_TWO:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer2 : 0;
                continue;
            case GOW_KEY_THREE:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer3 : 0;
                continue;
            case GOW_KEY_FOUR:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer4 : 0;
                continue;
            case GOW_KEY_FIVE:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer5 : 0;
                continue;
            case GOW_KEY_SIX:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer6 : 0;
                continue;
            case GOW_KEY_SEVEN:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer7 : 0;
                continue;
            case GOW_KEY_EIGHT:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer8 : 0;
                continue;
            case GOW_KEY_NINE:
                _studioEngineState->_state ^= e.isDown() ? SESS_Layer9 : 0;
                continue;
            case GOW_KEY_ARROW_DOWN:
                _selectionIndexDir = e.isPressed() ? 0.25f : 0;
                continue;
            case GOW_KEY_ARROW_UP:
                _selectionIndexDir = e.isPressed() ? -0.25f : 0;
                continue;
            case GOW_KEY_CARRIAGE_RETURN:
            {
                if (e.isDown())
                {
                    const std::vector<EntityDef*>& entityDescriptors = ENTITY_MAPPER.getEntityDescriptors();
                    EntityDef* entityDef = entityDescriptors[_selectionIndex];
                    if (entityDef->_key == 'ROBT')
                    {
                        _studioEngineState->_renderer.displayToast("Players can only be spawned by the server...");
                        continue;
                    }
                    
                    addEntity(entityDef);
                }
                continue;
            }
            case GOW_KEY_E:
            case GOW_KEY_ESCAPE:
            case GOW_KEY_BACK_SPACE:
            {
                if (e.isDown())
                {
                    SET_BIT(_studioEngineState->_state, SESS_DisplayEntities, false);
                }
                continue;
            }
            default:
                continue;
        }
    }
    
    const std::vector<EntityDef*>& entityDescriptors = ENTITY_MAPPER.getEntityDescriptors();
    int numEntityIndices = static_cast<int>(entityDescriptors.size()) - 1;
    _rawSelectionIndex = clamp(_rawSelectionIndex + _selectionIndexDir, 0, numEntityIndices);
    _selectionIndex = clamp(_rawSelectionIndex, 0, numEntityIndices);
}

void StudioInputManager::handleLoadMapDialogInput()
{
    std::vector<KeyboardEvent*>& keyboardEvents = INPUT_MGR.getKeyboardEvents();
    for (std::vector<KeyboardEvent *>::iterator i = keyboardEvents.begin(); i != keyboardEvents.end(); ++i)
    {
        KeyboardEvent& e = *(*i);
        switch (e._key)
        {
            case GOW_KEY_ARROW_DOWN:
                _selectionIndexDir = e.isPressed() ? 0.25f : 0;
                continue;
            case GOW_KEY_ARROW_UP:
                _selectionIndexDir = e.isPressed() ? -0.25f : 0;
                continue;
            case GOW_KEY_CARRIAGE_RETURN:
            {
                if (e.isDown())
                {
                    std::vector<MapDef>& maps = EntityLayoutMapper::getInstance().getMaps();
                    uint32_t map = maps[_selectionIndex]._key;
                    _studioEngineState->_world.loadMap(map);
                    onMapLoaded();
                    _studioEngineState->_state &= ~SESS_DisplayLoadMapDialog;
                }
            }
                continue;
            case GOW_KEY_ESCAPE:
            case GOW_KEY_BACK_SPACE:
                _studioEngineState->_state &= e.isDown() ? ~SESS_DisplayLoadMapDialog : 0;
                continue;
            default:
                continue;
        }
    }
    
    _rawSelectionIndex = clamp(_rawSelectionIndex + _selectionIndexDir, 0, 1);
    _selectionIndex = clamp(_rawSelectionIndex, 0, 1);
}

bool layerSort(Entity* l, Entity* r)
{
    TextureRegion ltr = ASSETS.findTextureRegion(l->getTextureMapping(), l->getStateTime());
    TextureRegion rtr = ASSETS.findTextureRegion(r->getTextureMapping(), r->getStateTime());
    
    return (ltr._layer > rtr._layer);
}

void StudioInputManager::onMapLoaded()
{
    _activeEntity = NULL;
    _lastActiveEntity = NULL;
    _entities.clear();
    
    std::vector<Entity*>& dynamicEntities = _studioEngineState->_world.getDynamicEntities();
    std::vector<Entity*>& staticEntities = _studioEngineState->_world.getStaticEntities();
    std::vector<Entity*>& layers = _studioEngineState->_world.getLayers();
    
    size_t size = dynamicEntities.size();
    size += staticEntities.size();
    size += layers.size();
    
    _entities.reserve(size);
    
    _entities.insert(_entities.end(), dynamicEntities.begin(), dynamicEntities.end());
    _entities.insert(_entities.end(), staticEntities.begin(), staticEntities.end());
    _entities.insert(_entities.end(), layers.begin(), layers.end());
    
    std::sort(_entities.begin(), _entities.end(), layerSort);
}

void StudioInputManager::onEntityAdded(Entity* e)
{
    _entities.push_back(e);
    std::sort(_entities.begin(), _entities.end(), layerSort);
}

void StudioInputManager::onEntityRemoved(Entity* e)
{
    for (std::vector<Entity*>::iterator i = _entities.begin(); i != _entities.end(); )
    {
        Entity* ie = (*i);
        if (ie == e)
        {
            i = _entities.erase(i);
            return;
        }
        else
        {
            ++i;
        }
    }
}

Entity* StudioInputManager::getEntityAtPosition(float x, float y)
{
    if (_lastActiveEntity && entityExistsAtPosition(_lastActiveEntity, x, y))
    {
        return _lastActiveEntity;
    }
    
    for (std::vector<Entity*>::iterator i = _entities.begin(); i != _entities.end(); ++i)
    {
        Entity* e = (*i);
        TextureRegion tr = ASSETS.findTextureRegion(e->getTextureMapping(), e->getStateTime());
        int layer = tr._layer;
        if (_studioEngineState->_state & (1 << (layer + SESS_LayerBitBegin)))
        {
            if (entityExistsAtPosition(e, x, y))
            {
                return e;
            }
        }
    }
    
    return NULL;
}

bool StudioInputManager::entityExistsAtPosition(Entity* e, float x, float y)
{
    float eX = e->getPosition().x;
    float eY = e->getPosition().y;
    float eW = e->getWidth();
    float eH = e->getHeight();
    Rektangle entityBounds = Rektangle(eX - eW / 2, eY - eH / 2, eW, eH);
    if (OverlapTester::isPointInRektangle(x, y, entityBounds))
    {
        return true;
    }
    
    return false;
}

Entity* StudioInputManager::addEntity(EntityDef* entityDef, int width, int height)
{
    float spawnX = clamp(CFG_MAIN._camWidth * _scrollValue / 2 + _cursor.x(), entityDef->_width / 2.0f, FLT_MAX);
    float spawnY = clamp(CFG_MAIN._camHeight * _scrollValue / 2 + _cursor.y(), entityDef->_height / 2.0f, FLT_MAX);
    EntityInstanceDef eid(_entityIDManager->getNextStaticEntityID(), entityDef->_key, floor(spawnX), floor(spawnY), width, height);
    Entity* e = ENTITY_MAPPER.createEntityFromDef(entityDef, &eid, false);
    _studioEngineState->_world.addEntity(e);
    _lastActiveEntity = e;
    onEntityAdded(e);
    _studioEngineState->_state &= ~SESS_DisplayEntities;
    
    return e;
}

void StudioInputManager::updateCamera()
{
    int w = CFG_MAIN._camWidth * _scrollValue;
    int h = CFG_MAIN._camHeight * _scrollValue;
    float topPan = h * 0.95f;
    float bottomPan = h * 0.05f;
    float rightPan = w * 0.95f;
    float leftPan = w * 0.05f;
    
    if (_lastScrollValue != _scrollValue)
    {
        CURSOR_CONVERTER.setMatrixSize(CFG_MAIN._camWidth * _lastScrollValue, CFG_MAIN._camHeight * _lastScrollValue);
        Vector2& rc = _cursorPosition;
        Vector2 c = CURSOR_CONVERTER.convert(rc);
        
        int dw = CFG_MAIN._camWidth * _lastScrollValue;
        int dh = CFG_MAIN._camHeight * _lastScrollValue;
        
        float xFactor = c.x() / dw;
        float yFactor = c.y() / dh;
        
        Vector2 center = _cursor;
        center.add(dw * xFactor, dh * yFactor);
        center.sub(w * xFactor, h * yFactor);
        
        _cursor = center;
        
        _rawScrollValue = _scrollValue;
        _lastScrollValue = _scrollValue;
        CURSOR_CONVERTER.setMatrixSize(CFG_MAIN._camWidth * _scrollValue, CFG_MAIN._camHeight * _scrollValue);
    }
    
    {
        /// Update Camera based on mouse being near edges
        Vector2& rc = _cursorPosition;
        Vector2 c = CURSOR_CONVERTER.convert(rc);
        if ((_hasTouchedScreen && c.y() > topPan) || _isPanningUp)
        {
            _cursor.add(0, h / CFG_MAIN._camHeight / 2.0f);
            _activeEntityCursor.add(0, h / CFG_MAIN._camHeight / 2.0f);
        }
        if ((_hasTouchedScreen && c.y() < bottomPan) || _isPanningDown)
        {
            _cursor.sub(0, h / CFG_MAIN._camHeight / 2.0f);
            _activeEntityCursor.sub(0, h / CFG_MAIN._camHeight / 2.0f);
        }
        if ((_hasTouchedScreen && c.x() > rightPan) || _isPanningRight)
        {
            _cursor.add(w / CFG_MAIN._camWidth / 2.0f, 0);
            _activeEntityCursor.add(w / CFG_MAIN._camWidth / 2.0f, 0);
        }
        if ((_hasTouchedScreen && c.x() < leftPan) || _isPanningLeft)
        {
            _cursor.sub(w / CFG_MAIN._camWidth / 2.0f, 0);
            _activeEntityCursor.sub(w / CFG_MAIN._camWidth / 2.0f, 0);
        }
    }
}

void StudioInputManager::resetCamera()
{
    int w = CFG_MAIN._camWidth * _scrollValue;
    int h = CFG_MAIN._camHeight * _scrollValue;
    _rawScrollValue = 1;
    CURSOR_CONVERTER.setMatrixSize(w, h);
    _cursor.set(0, 0);
}

StudioInputManager::StudioInputManager() :
_entityIDManager(static_cast<EntityIDManager*>(INSTANCE_MGR.get(INSTANCE_ENTITY_ID_MANAGER_STUDIO))),
_downCursor(),
_dragCursor(),
_deltaCursor(),
_cursor(),
_upCursor(),
_cursorPosition(),
_inputState(SIMS_NONE),
_isControl(false),
_rawScrollValue(1),
_scrollValue(1),
_lastScrollValue(1),
_isPanningUp(false),
_isPanningDown(false),
_isPanningRight(false),
_isPanningLeft(false),
_selectionIndex(0),
_rawSelectionIndex(0),
_selectionIndexDir(0),
_activeEntity(NULL),
_lastActiveEntity(NULL),
_activeEntityCursor(),
_studioEngineState(StudioEngineState::getInstance()),
_hasTouchedScreen(false)
{
    resetCamera();
}

StudioInputManager::~StudioInputManager()
{
    // Empty
}
