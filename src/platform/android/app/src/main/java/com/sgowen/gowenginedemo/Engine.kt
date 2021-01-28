package com.sgowen.gowenginedemo

import android.content.res.AssetManager

class Engine
{
    init
    {
        init()
    }

    external fun createDeviceDependentResources(assetManager: AssetManager)
    external fun onWindowSizeChanged(screenWidth: Int, screenHeight: Int, cursorWidth: Int, cursorHeight: Int)
    external fun releaseDeviceDependentResources()
    external fun onResume()
    external fun onPause()
    external fun update(deltaTime: Double): Int
    external fun render()
    external fun onCursorDown(x: Float, y: Float)
    external fun onCursorDragged(x: Float, y: Float)
    external fun onCursorUp(x: Float, y: Float)

    private external fun init()

    companion object
    {
        init
        {
            System.loadLibrary("native-lib")
        }
    }
}
