package com.sgowen.gowenginedemo

import android.content.res.AssetManager

class Engine(framesPerSecond: Int)
{
    init
    {
        init(framesPerSecond)
    }

    external fun createDeviceDependentResources(assetManager: AssetManager): Void
    external fun onWindowSizeChanged(screenWidth: Int, screenHeight: Int, cursorWidth: Int, cursorHeight: Int): Void
    external fun releaseDeviceDependentResources(): Void
    external fun onResume(): Void
    external fun onPause(): Void
    external fun update(deltaTime: Double): Void
    external fun render(): Void
    external fun onCursorDown(x: Float, y: Float, isAlt: Boolean): Void
    external fun onCursorDragged(x: Float, y: Float, isAlt: Boolean): Void
    external fun onCursorUp(x: Float, y: Float, isAlt: Boolean): Void

    private external fun init(framesPerSecond: Int): Void

    companion object
    {
        init
        {
            System.loadLibrary("native-lib")
        }
    }
}
