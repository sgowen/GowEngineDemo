package com.sgowen.gowenginedemo

import android.annotation.SuppressLint
import android.content.Context
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.os.SystemClock
import android.view.MotionEvent
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class MainActivity : AppCompatActivity()
{
    override fun onCreate(savedInstanceState: Bundle?)
    {
        super.onCreate(savedInstanceState)

        engine = Engine()

        setContentView(MyGLSurfaceView(engine, this))
    }

    override fun onDestroy()
    {
        engine.releaseDeviceDependentResources()

        super.onDestroy()
    }

    override fun onWindowFocusChanged(hasFocus: Boolean)
    {
        super.onWindowFocusChanged(hasFocus)

        if (hasFocus)
        {
            hideSystemUI()
        }
    }

    private lateinit var engine: Engine

    private fun hideSystemUI()
    {
        // Enables regular immersive mode.
        // For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
        // Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        window.decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_IMMERSIVE
                // Set the content to appear under the system bars so that the
                // content doesn't resize when the system bars hide and show.
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                // Hide the nav bar and status bar
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_FULLSCREEN)
    }

    private class MyGLSurfaceView(val engine: Engine, context: Context) : GLSurfaceView(context)
    {
        init
        {
            setEGLContextClientVersion(3)

            setRenderer(MyGLRenderer(context, engine))
        }

        @SuppressLint("ClickableViewAccessibility")
        override fun onTouchEvent(e: MotionEvent): Boolean
        {
            val x: Float = e.x
            val y: Float = e.y

            when (e.action)
            {
                MotionEvent.ACTION_DOWN ->
                {
                    engine.onCursorDown(x, y, false)
                }
                MotionEvent.ACTION_MOVE ->
                {
                    engine.onCursorDragged(x, y, false)
                }
                MotionEvent.ACTION_UP ->
                {
                    engine.onCursorUp(x, y, false)
                }
            }

            return true
        }
    }

    private class MyGLRenderer(val context: Context, val engine: Engine) : GLSurfaceView.Renderer
    {
        override fun onSurfaceCreated(unused: GL10, config: EGLConfig)
        {
            engine.createDeviceDependentResources(assetManager = context.resources.assets)
        }

        override fun onDrawFrame(unused: GL10)
        {
            currTimePick = SystemClock.uptimeMillis()
            realTimeElapsed = if (lastRealTimeMeasurement > 0)
            {
                (currTimePick - lastRealTimeMeasurement).toDouble()
            }
            else
            {
                smoothedDeltaRealTime
            }

            movAverageDeltaTime = (realTimeElapsed + movAverageDeltaTime * (movAveragePeriod - 1)) / movAveragePeriod
            smoothedDeltaRealTime += (movAverageDeltaTime - smoothedDeltaRealTime) * smoothFactor

            lastRealTimeMeasurement = currTimePick

            deltaTime = smoothedDeltaRealTime / 1000
            engine.update(deltaTime)
            engine.render()
        }

        override fun onSurfaceChanged(unused: GL10, width: Int, height: Int)
        {
            engine.onWindowSizeChanged(width, height, width, height)
        }

        private var deltaTime: Double = 0.0
        private var currTimePick: Long = 0
        private var realTimeElapsed: Double = 0.0
        private val movAveragePeriod = 40f
        private val smoothFactor = 0.1f
        private var smoothedDeltaRealTime = 1 / 30.0
        private var movAverageDeltaTime = smoothedDeltaRealTime
        private var lastRealTimeMeasurement: Long = 0
    }
}
