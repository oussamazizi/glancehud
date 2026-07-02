package com.glancehud.example

import android.app.Activity
import android.os.Bundle
import android.view.Gravity
import android.view.ViewGroup
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView
import com.glancehud.GlanceConfig
import com.glancehud.GlanceHud
import com.glancehud.service.GlancePersistentHud

/**
 * Minimal demo: grant the overlay permission, then start/stop GlanceHUD.
 * The overlay itself floats above this (and any other) app.
 */
class MainActivity : Activity() {

    private var running = false
    private var persistent = false
    private lateinit var toggleButton: Button
    private lateinit var status: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        GlanceHud.initialize(this, GlanceConfig(sampleIntervalMs = 500, opacity = 0.85f))

        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setPadding(48, 48, 48, 48)
            layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT,
            )
        }

        val title = TextView(this).apply {
            text = "GlanceHUD demo"
            textSize = 22f
        }

        status = TextView(this).apply {
            text = "Overlay stopped"
            textSize = 14f
            setPadding(0, 24, 0, 24)
        }

        val permissionButton = Button(this).apply {
            text = "1) Grant overlay permission"
            setOnClickListener { GlanceHud.requestOverlayPermission(this@MainActivity) }
        }

        toggleButton = Button(this).apply {
            text = "2) Start overlay"
            setOnClickListener { toggle() }
        }

        val persistentButton = Button(this).apply {
            text = "3) Start persistent (survives background)"
            setOnClickListener { startPersistent() }
        }

        root.addView(title)
        root.addView(status)
        root.addView(permissionButton)
        root.addView(toggleButton)
        root.addView(persistentButton)
        setContentView(root)
    }

    private fun startPersistent() {
        if (!GlanceHud.hasOverlayPermission(this)) {
            status.text = "Grant the overlay permission first (button 1)."
            GlanceHud.requestOverlayPermission(this)
            return
        }
        GlancePersistentHud.start(this)
        running = true
        persistent = true
        toggleButton.text = "Stop overlay"
        status.text = "Persistent overlay running — try pressing Home."
    }

    private fun toggle() {
        if (!GlanceHud.hasOverlayPermission(this)) {
            status.text = "Grant the overlay permission first (button 1)."
            GlanceHud.requestOverlayPermission(this)
            return
        }
        running = !running
        if (running) {
            GlanceHud.start()
            persistent = false
            toggleButton.text = "Stop overlay"
            status.text = "Overlay running — drag it, tap to collapse."
        } else {
            stopOverlay()
            toggleButton.text = "2) Start overlay"
            status.text = "Overlay stopped"
        }
    }

    private fun stopOverlay() {
        if (persistent) {
            GlancePersistentHud.stop(this)
            persistent = false
        } else {
            GlanceHud.stop()
        }
    }

    override fun onDestroy() {
        stopOverlay()
        super.onDestroy()
    }
}
