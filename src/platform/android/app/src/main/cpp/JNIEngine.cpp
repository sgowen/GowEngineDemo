#include <jni.h>
#include <assert.h>

#include "Engine.hpp"
#include "MainEngineController.hpp"
#include "android/JNIAndroidAssetHandler.hpp"

Engine* _engine = NULL;
MainEngineController _mainEngineController;

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_createDeviceDependentResources(JNIEnv *env, jobject thiz, jobject asset_manager)
{
    JNIAndroidAssetHandler::create(env, asset_manager);

    _engine->createDeviceDependentResources();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_onWindowSizeChanged(JNIEnv *env, jobject thiz, jint screen_width, jint screen_height, jint cursor_width, jint cursor_height)
{
    _engine->onWindowSizeChanged(screen_width, screen_height, cursor_width, cursor_height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_releaseDeviceDependentResources(JNIEnv *env, jobject thiz)
{
    _engine->releaseDeviceDependentResources();

    JNIAndroidAssetHandler::destroy();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_onResume(JNIEnv *env, jobject thiz)
{
    _engine->onResume();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_onPause(JNIEnv *env, jobject thiz)
{
    _engine->onPause();
}

extern "C"
JNIEXPORT int JNICALL
Java_com_sgowen_gowenginedemo_Engine_update(JNIEnv *env, jobject thiz, jdouble delta_time)
{
    return _engine->update(delta_time);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_render(JNIEnv *env, jobject thiz)
{
    _engine->render();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_onCursorDown(JNIEnv *env, jobject thiz, jfloat x, jfloat y)
{
    _engine->onCursorDown(x, y);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_onCursorDragged(JNIEnv *env, jobject thiz, jfloat x, jfloat y)
{
    _engine->onCursorDragged(x, y);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_onCursorUp(JNIEnv *env, jobject thiz, jfloat x, jfloat y)
{
    _engine->onCursorUp(x, y);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sgowen_gowenginedemo_Engine_init(JNIEnv *env, jobject thiz)
{
    assert(_engine == NULL);
    
    _engine = new Engine(_mainEngineController);
}
