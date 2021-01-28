//
//  StudioInputManager.hpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 1/4/18.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "Vector2.hpp"

#include <string>
#include <vector>

class EntityIDManager;
class StudioEngineState;
class Entity;
struct EntityDef;

enum StudioInputManagerState
{
    SIMS_NONE = 0,
    SIMS_ESCAPE
};

class StudioInputManager
{
    friend class StudioRenderer;
    
public:
    static void create();
    static StudioInputManager* getInstance();
    static void destroy();
    
    int getMenuState();
    void update();
    
private:
    static StudioInputManager* s_instance;
    static uint32_t s_testMap;
    
    EntityIDManager* _entityIDManager;
    Vector2 _downCursor;
    Vector2 _dragCursor;
    Vector2 _deltaCursor;
    Vector2 _cursor;
    Vector2 _upCursor;
    Vector2 _cursorPosition;
    int _inputState;
    bool _isControl;
    float _rawScrollValue;
    int _scrollValue;
    int _lastScrollValue;
    bool _isPanningUp;
    bool _isPanningDown;
    bool _isPanningRight;
    bool _isPanningLeft;
    int _selectionIndex;
    float _rawSelectionIndex;
    float _selectionIndexDir;
    std::vector<Entity*> _entities;
    Entity* _activeEntity;
    Entity* _lastActiveEntity;
    Vector2 _activeEntityCursor;
    Vector2 _activeEntityDeltaCursor;
    StudioEngineState* _studioEngineState;
    bool _hasTouchedScreen;
    
    void updateCamera();
    void resetCamera();
    
    void handleDefaultInput();
    void handleEntitiesInput();
    void handleLoadMapDialogInput();
    
    void onMapLoaded();
    void onEntityAdded(Entity* e);
    void onEntityRemoved(Entity* e);
    Entity* getEntityAtPosition(float x, float y);
    bool entityExistsAtPosition(Entity* e, float x, float y);
    Entity* addEntity(EntityDef* entityDef, int width = 0, int height = 0);
    
    StudioInputManager();
    ~StudioInputManager();
    StudioInputManager(const StudioInputManager&);
    StudioInputManager& operator=(const StudioInputManager&);
};
