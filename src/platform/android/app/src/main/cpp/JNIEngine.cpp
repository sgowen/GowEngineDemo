#include <jni.h>

#include "Engine.hpp"
#include "MainEngineController.hpp"
#include "android/JNIAndroidAssetHandler.hpp"

Engine* _engine = NULL;
MainEngineController* _mainEngineController = NULL;

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_createDeviceDependentResources(JNIEnv *env, jobject thiz, jobject asset_manager)
{
    JNIAndroidAssetHandler::create(env, asset_manager);

    _engine->createDeviceDependentResources();

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_onWindowSizeChanged(JNIEnv *env, jobject thiz, jint screen_width, jint screen_height, jint cursor_width, jint cursor_height)
{
    _engine->onWindowSizeChanged(screen_width, screen_height, cursor_width, cursor_height);

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_releaseDeviceDependentResources(JNIEnv *env, jobject thiz)
{
    _engine->releaseDeviceDependentResources();

    JNIAndroidAssetHandler::destroy();

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_onResume(JNIEnv *env, jobject thiz)
{
    _engine->onResume();

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_onPause(JNIEnv *env, jobject thiz)
{
    _engine->onPause();

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_update(JNIEnv *env, jobject thiz, jdouble delta_time)
{
    _engine->update(delta_time);

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_render(JNIEnv *env, jobject thiz)
{
    _engine->render();

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_onCursorDown(JNIEnv *env, jobject thiz, jfloat x, jfloat y, jboolean is_alt)
{
    _engine->onCursorDown(x, y, is_alt);

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_onCursorDragged(JNIEnv *env, jobject thiz, jfloat x, jfloat y, jboolean is_alt)
{
    _engine->onCursorDragged(x, y, is_alt);

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_onCursorUp(JNIEnv *env, jobject thiz, jfloat x, jfloat y, jboolean is_alt)
{
    _engine->onCursorUp(x, y, is_alt);

    return 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sgowen_gowenginedemo_Engine_init(JNIEnv *env, jobject thiz)
{
    if (_engine == NULL)
    {
        _mainEngineController = new MainEngineController();
        _engine = new Engine(_mainEngineController);
    }

    return 0;
}
