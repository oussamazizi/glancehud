package com.glancehud

import android.content.Context
import android.graphics.PixelFormat
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.provider.Settings
import android.util.Log
import android.view.Choreographer
import android.view.Gravity
import android.view.MotionEvent
import android.view.WindowManager
import kotlin.math.abs

/**
 * Owns the floating overlay window: its lifecycle, the per-frame update loop,
 * and drag / tap-to-collapse gestures.
 *
 * All window operations run on the main thread (WindowManager requires it),
 * hence the [mainHandler] posts.
 */
internal class GlanceOverlayManager(
    private val context: Context,
    private val config: GlanceConfig,
) {
    private val windowManager =
        context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
    private val mainHandler = Handler(Looper.getMainLooper())
    private val choreographer = Choreographer.getInstance()

    private var view: GlanceOverlayView? = null
    private var params: WindowManager.LayoutParams? = null
    private var running = false

    fun show() = mainHandler.post { showInternal() }

    fun hide() = mainHandler.post {
        stopLoop()
        view?.let { runCatching { windowManager.removeView(it) } }
        view = null
        params = null
    }

    fun setOpacity(opacity: Float) = mainHandler.post {
        view?.setOpacityFactor(opacity.coerceIn(0f, 1f))
    }

    fun setPosition(x: Int, y: Int) = mainHandler.post {
        val p = params ?: return@post
        p.x = x
        p.y = y
        view?.let { windowManager.updateViewLayout(it, p) }
    }

    private fun showInternal() {
        if (view != null) return
        if (!Settings.canDrawOverlays(context)) {
            Log.w(TAG, "Overlay permission not granted; call requestOverlayPermission() first.")
            return
        }

        val overlay = GlanceOverlayView(context).apply {
            setOpacityFactor(config.opacity)
            collapsed = config.startCollapsed
        }
        val layoutParams = createParams()
        attachTouch(overlay, layoutParams)

        windowManager.addView(overlay, layoutParams)
        view = overlay
        params = layoutParams
        startLoop()
    }

    private fun createParams(): WindowManager.LayoutParams {
        val type = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
        } else {
            @Suppress("DEPRECATION")
            WindowManager.LayoutParams.TYPE_PHONE
        }
        return WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            type,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE or
                WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL or
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
            PixelFormat.TRANSLUCENT,
        ).apply {
            gravity = Gravity.TOP or Gravity.START
            x = DEFAULT_MARGIN
            y = DEFAULT_MARGIN
        }
    }

    private fun attachTouch(overlay: GlanceOverlayView, p: WindowManager.LayoutParams) {
        var startX = 0
        var startY = 0
        var touchRawX = 0f
        var touchRawY = 0f
        var dragged = false

        overlay.setOnTouchListener { _, event ->
            when (event.action) {
                MotionEvent.ACTION_DOWN -> {
                    startX = p.x
                    startY = p.y
                    touchRawX = event.rawX
                    touchRawY = event.rawY
                    dragged = false
                    true
                }
                MotionEvent.ACTION_MOVE -> {
                    val dx = (event.rawX - touchRawX).toInt()
                    val dy = (event.rawY - touchRawY).toInt()
                    if (abs(dx) > TOUCH_SLOP || abs(dy) > TOUCH_SLOP) dragged = true
                    p.x = startX + dx
                    p.y = startY + dy
                    windowManager.updateViewLayout(overlay, p)
                    true
                }
                MotionEvent.ACTION_UP -> {
                    if (!dragged) {
                        overlay.toggleCollapsed()
                        windowManager.updateViewLayout(overlay, p)
                    }
                    true
                }
                else -> false
            }
        }
    }

    private fun startLoop() {
        if (running) return
        running = true
        choreographer.postFrameCallback(frameCallback)
    }

    private fun stopLoop() {
        running = false
        choreographer.removeFrameCallback(frameCallback)
    }

    private val frameCallback = object : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            val data = GlanceHud.tick()
            view?.setData(data)
            if (running) choreographer.postFrameCallback(this)
        }
    }

    private companion object {
        const val TAG = "GlanceHud"
        const val TOUCH_SLOP = 8
        const val DEFAULT_MARGIN = 24
    }
}
