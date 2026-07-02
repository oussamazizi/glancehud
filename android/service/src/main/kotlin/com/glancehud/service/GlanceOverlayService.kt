package com.glancehud.service

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.content.pm.ServiceInfo
import android.os.Build
import android.os.IBinder
import com.glancehud.GlanceHud

/**
 * Hosts the overlay from a foreground service so it survives the host app going
 * to the background or being killed. Started via [GlancePersistentHud.start].
 *
 * A foreground service needs an ongoing notification; on API 34+ it must also
 * declare a foreground service type (here: "special use", the correct bucket for
 * a developer diagnostics overlay).
 *
 * The overlay window itself is owned by [GlanceHud] (a system-level
 * TYPE_APPLICATION_OVERLAY window, independent of this service's lifecycle);
 * this service only keeps the process alive and drives show/hide.
 */
class GlanceOverlayService : Service() {

    override fun onCreate() {
        super.onCreate()
        startAsForeground()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        GlanceHud.start()  // ensure sampling is running
        GlanceHud.show()
        return START_STICKY
    }

    override fun onDestroy() {
        GlanceHud.stop()
        super.onDestroy()
    }

    override fun onBind(intent: Intent?): IBinder? = null

    private fun startAsForeground() {
        val notificationManager = getSystemService(NotificationManager::class.java)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                CHANNEL_ID,
                "GlanceHUD overlay",
                NotificationManager.IMPORTANCE_LOW,
            )
            notificationManager.createNotificationChannel(channel)
        }

        val builder = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Notification.Builder(this, CHANNEL_ID)
        } else {
            @Suppress("DEPRECATION")
            Notification.Builder(this)
        }
        val notification = builder
            .setContentTitle("GlanceHUD")
            .setContentText("Performance overlay running")
            .setSmallIcon(android.R.drawable.ic_menu_info_details)
            .setOngoing(true)
            .build()

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
            startForeground(NOTIFICATION_ID, notification, ServiceInfo.FOREGROUND_SERVICE_TYPE_SPECIAL_USE)
        } else {
            startForeground(NOTIFICATION_ID, notification)
        }
    }

    private companion object {
        const val CHANNEL_ID = "glancehud_overlay"
        const val NOTIFICATION_ID = 0x6C41  // arbitrary, unlikely to clash
    }
}
