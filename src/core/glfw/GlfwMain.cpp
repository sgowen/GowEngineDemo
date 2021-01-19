//
//  GlfwMain.cpp
//  GowEngineDemo
//
//  Created by Stephen Gowen on 11/16/17.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "GlfwMain.hpp"

#include "EngineController.hpp"

#include "PlatformMacros.hpp"
#include "Engine.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <cstring>

int GlfwMain::joysticks[GLFW_JOYSTICK_LAST + 1];
int GlfwMain::joystick_count = 0;

GlfwMain* GlfwMain::getInstance()
{
    static GlfwMain instance = GlfwMain();
    return &instance;
}

void GlfwMain::error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void GlfwMain::joystick_callback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        joysticks[joystick_count++] = jid;
    }
    else if (event == GLFW_DISCONNECTED)
    {
        int i;

        for (i = 0; i < joystick_count; ++i)
        {
            if (joysticks[i] == jid)
            {
                break;
            }
        }

        for (i = i + 1; i < joystick_count; ++i)
        {
            joysticks[i - 1] = joysticks[i];
        }

        joystick_count--;
    }
}

void GlfwMain::mouse_scroll_callback(GLFWwindow* window, double x, double y)
{
//    CURSOR_INPUT_MANAGER->onScroll(-y);
}

static bool isDown = false;
static bool isAlt = false;

void GlfwMain::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    static double x;
    static double y;

    glfwGetCursorPos(window, &x, &y);
    
    isAlt = button == GLFW_MOUSE_BUTTON_2;
    
    switch (action)
    {
        case GLFW_PRESS:
//            CURSOR_INPUT_MANAGER->onInput(CursorEventType_DOWN, x, y, isAlt);
            isDown = true;
            break;
        case GLFW_RELEASE:
            isDown = false;
//            CURSOR_INPUT_MANAGER->onInput(CursorEventType_UP, x, y, isAlt);
            break;
        default:
            break;
    }
}

void GlfwMain::mouse_cursor_pos_callback(GLFWwindow*, double x, double y)
{
//    CURSOR_INPUT_MANAGER->setCursorPosition(x, y);
    
    if (isDown)
    {
//        CURSOR_INPUT_MANAGER->onInput(CursorEventType_DRAGGED, x, y, isAlt);
    }
}

void GlfwMain::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
//    KEYBOARD_INPUT_MANAGER->onInput(key, action == GLFW_RELEASE);
}

int GlfwMain::exec(EngineController* engineController)
{
    if (!engineController)
    {
        exit(EXIT_FAILURE);
    }

    memset(joysticks, 0, sizeof(joysticks));

    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWmonitor* monitor = NULL;

    int width = 800;
    int height = 480;

#if !defined(_DEBUG) && !defined(DEBUG)
#define LAUNCH_FULL_SCREEN
#endif

#ifdef LAUNCH_FULL_SCREEN
    /// Launch directly into full screen
    monitor = glfwGetPrimaryMonitor();

    if (monitor)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        width  = mode->width;
        height = mode->height;
    }
#endif

    window = glfwCreateWindow(width, height, "Project Dante", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid)
    {
        if (glfwJoystickPresent(jid))
        {
            joysticks[joystick_count++] = jid;
        }
    }

    glfwSetJoystickCallback(GlfwMain::joystick_callback);
    glfwSetMouseButtonCallback(window, GlfwMain::mouse_button_callback);
    glfwSetCursorPosCallback(window, GlfwMain::mouse_cursor_pos_callback);
    glfwSetScrollCallback(window, GlfwMain::mouse_scroll_callback);
    glfwSetKeyCallback(window, GlfwMain::key_callback);

    glfwMakeContextCurrent(window);
#if IS_WINDOWS
    gladLoadGL(glfwGetProcAddress);
#endif
    glfwSwapInterval(1);

#if IS_LINUX
    glewInit();
#endif

    double lastTime = 0;

    Engine* engineP = new Engine(engineController);
    
    Engine& engine = *engineP;
    engine.createDeviceDependentResources();

    int glWidth = 0, glHeight = 0;

    while (!glfwWindowShouldClose(window))
    {
        double deltaTime = glfwGetTime() - lastTime;
        lastTime = glfwGetTime();
        
        glfwPollEvents();
        
        glfwGetFramebufferSize(window, &width, &height);
        if (width != glWidth || height != glHeight)
        {
            int screenWidth = 0;
            int screenHeight = 0;
            glfwGetWindowSize(window, &screenWidth, &screenHeight);
            
            engine.onWindowSizeChanged(width, height, screenWidth, screenHeight);

            glWidth = width;
            glHeight = height;
        }

        for (int i = 0; i < joystick_count; ++i)
        {
            int j, axis_count, button_count;
            const float* axes;
            const unsigned char* buttons;

            axes = glfwGetJoystickAxes(joysticks[i], &axis_count);
            buttons = glfwGetJoystickButtons(joysticks[i], &button_count);

            float stickLeftX = 0;
            float stickLeftY = 0;
            float stickRightX = 0;
            float stickRightY = 0;
            float triggerLeft = 0;
            float triggerRight = 0;

            for (j = 0; j < axis_count; ++j)
            {
                switch (j)
                {
                    case 0:
                        stickLeftX = axes[j];
                        break;
                    case 1:
                        stickLeftY = axes[j];
                        break;
                    case 2:
                        stickRightX = axes[j];
                        break;
                    case 3:
                        triggerLeft = axes[j];
                        break;
                    case 4:
                        triggerRight = axes[j];
                        break;
                    case 5:
                        stickRightY = axes[j];
                    default:
                        break;
                }
            }

//            GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_STICK_LEFT, i, stickLeftX, stickLeftY);
//            GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_STICK_RIGHT, i, stickRightX, stickRightY);
//            GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_TRIGGER, i, triggerLeft, triggerRight);

            for (j = 0; j < button_count; ++j)
            {
                switch (j)
                {
                    case 0:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_X_BUTTON, i, buttons[j], 0);
                        break;
                    case 1:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_A_BUTTON, i, buttons[j], 0);
                        break;
                    case 2:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_B_BUTTON, i, buttons[j], 0);
                        break;
                    case 3:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_Y_BUTTON, i, buttons[j], 0);
                        break;
                    case 4:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_BUMPER_LEFT, i, buttons[j], 0);
                        break;
                    case 5:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_BUMPER_RIGHT, i, buttons[j], 0);
                        break;
                    case 8:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_BACK_BUTTON, i, buttons[j], 0);
                        break;
                    case 9:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_START_BUTTON, i, buttons[j], 0);
                        break;
                    case 14:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_D_PAD_UP, i, buttons[j], 0);
                        break;
                    case 15:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_D_PAD_RIGHT, i, buttons[j], 0);
                        break;
                    case 16:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_D_PAD_DOWN, i, buttons[j], 0);
                        break;
                    case 17:
//                        GAME_PAD_INPUT_MANAGER->onInput(GamePadEventType_D_PAD_LEFT, i, buttons[j], 0);
                        break;
                    default:
                        break;
                }
            }
        }

//        int requestedAction = engine.getRequestedAction();
//
//        switch (requestedAction)
//        {
//            case REQUESTED_ACTION_EXIT:
//                glfwSetWindowShouldClose(window, GLFW_TRUE);
//                break;
//            case REQUESTED_ACTION_UPDATE:
//                break;
//            default:
//                engine.clearRequestedAction();
//                break;
//        }

        engine.update(deltaTime);
        engine.render();

        glfwSwapBuffers(window);
    }

    engine.releaseDeviceDependentResources();
    
    delete engineP;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

GlfwMain::GlfwMain()
{
    // Empty
}

GlfwMain::~GlfwMain()
{
    // Empty
}
