package com.glancehud

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.provider.Settings
import androidx.core.content.ContextCompat

/**
 * Public entry point for GlanceHUD on Android.
 *
 * Typical usage:
 * ```
 * GlanceHud.initialize(context)
 * if (!GlanceHud.hasOverlayPermission(context)) GlanceHud.requestOverlayPermission(activity)
 * GlanceHud.start()
 * // ...
 * GlanceHud.stop()
 * ```
 *
 * The heavy lifting (sampling thread, FPS math) happens in the shared C++ core;
 * this object only bridges to it and manages the native overlay window.
 */
object GlanceHud {

    init {
        System.loadLibrary("glancehud")
    }

    private var manager: GlanceOverlayManager? = null
    private var appContext: Context? = null
    private var config: GlanceConfig = GlanceConfig()
    private var serviceMode = false

    /** Configure the engine and prepare the overlay. Call once, early. */
    fun initialize(context: Context, config: GlanceConfig = GlanceConfig()) {
        this.config = config
        this.appContext = context.applicationContext
        nativeInitialize(config.sampleIntervalMs, config.opacity, config.autoShow, config.startCollapsed)
        manager = GlanceOverlayManager(context.applicationContext, config)
    }

    /** Start sampling; shows the overlay if [GlanceConfig.autoShow] is true. */
    fun start() {
        nativeStart()
        if (config.autoShow) manager?.show()
    }

    /**
     * Start sampling with the overlay hosted by a foreground service, so it
     * survives the host app going to the background or being killed.
     * Requires the overlay permission (see [hasOverlayPermission]).
     */
    fun startPersistent(context: Context) {
        nativeStart()
        serviceMode = true
        ContextCompat.startForegroundService(
            context,
            Intent(context, GlanceOverlayService::class.java),
        )
    }

    /** Stop sampling and remove the overlay (whichever mode was used). */
    fun stop() {
        if (serviceMode) {
            appContext?.let { it.stopService(Intent(it, GlanceOverlayService::class.java)) }
            serviceMode = false
        } else {
            manager?.hide()
        }
        nativeStop()
    }

    /** The active configuration (used by the overlay service). */
    internal fun currentConfig(): GlanceConfig = config

    fun show() = manager?.show()
    fun hide() = manager?.hide()

    fun setOpacity(opacity: Float) = manager?.setOpacity(opacity)
    fun setPosition(x: Int, y: Int) = manager?.setPosition(x, y)

    /** Whether the "draw over other apps" permission is granted. */
    fun hasOverlayPermission(context: Context): Boolean = Settings.canDrawOverlays(context)

    /** Open the system screen to grant the overlay permission. */
    fun requestOverlayPermission(activity: Activity) {
        val intent = Intent(
            Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
            Uri.parse("package:${activity.packageName}"),
        )
        activity.startActivity(intent)
    }

    /** Called each frame by the overlay loop: feeds FPS and reads the snapshot. */
    internal fun tick(): DoubleArray {
        nativeOnFrame()
        return nativeSnapshot()
    }

    // --- JNI bridge (implemented in glance_jni.cpp) --------------------------
    private external fun nativeInitialize(
        intervalMs: Int,
        opacity: Float,
        autoShow: Boolean,
        startCollapsed: Boolean,
    )

    private external fun nativeStart()
    private external fun nativeStop()
    private external fun nativeOnFrame()
    private external fun nativeSnapshot(): DoubleArray
}

/** User-facing configuration, mirrors the C++ `glance::Config`. */
data class GlanceConfig(
    val sampleIntervalMs: Int = 500,
    val opacity: Float = 0.85f,
    val autoShow: Boolean = true,
    val startCollapsed: Boolean = false,
)

/** Fixed layout of the DoubleArray returned by nativeSnapshot(). */
internal object SnapshotIndex {
    const val SEQUENCE = 0
    const val FPS = 1
    const val FRAME_TIME = 2
    const val CPU = 3
    const val RAM = 4
    const val GPU = 5
    const val NET_UP = 6
    const val NET_DOWN = 7
    const val SIZE = 8
}
