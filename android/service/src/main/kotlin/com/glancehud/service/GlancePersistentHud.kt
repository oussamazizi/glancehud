package com.glancehud.service

import android.content.Context
import android.content.Intent
import androidx.core.content.ContextCompat
import com.glancehud.GlanceHud

/**
 * Optional entry point for running the overlay from a foreground service, so it
 * stays alive when the host app is backgrounded or killed.
 *
 * Available only when the `:service` module is on the classpath — pull it in
 * alongside the core library:
 * ```
 * dependencies {
 *     implementation(project(":"))        // or the published core artifact
 *     implementation(project(":service")) // this opt-in add-on
 * }
 * ```
 *
 * Usage (after [GlanceHud.initialize] and granting the overlay permission):
 * ```
 * GlancePersistentHud.start(context)
 * // ...
 * GlancePersistentHud.stop(context)
 * ```
 */
object GlancePersistentHud {

    /** Start the overlay hosted by the foreground service. */
    fun start(context: Context) {
        ContextCompat.startForegroundService(
            context,
            Intent(context, GlanceOverlayService::class.java),
        )
    }

    /** Stop the foreground service and remove the overlay. */
    fun stop(context: Context) {
        context.stopService(Intent(context, GlanceOverlayService::class.java))
    }
}
