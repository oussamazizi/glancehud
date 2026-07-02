package com.glancehud

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RectF
import android.graphics.Typeface
import android.view.View

/**
 * The overlay's visual: a translucent rounded panel that lists the live metrics,
 * or a small handle when collapsed.
 *
 * It is a plain [View] drawn with [Canvas] — no XML, no extra dependencies —
 * matching GlanceHUD's "lightweight" goal. Data arrives via [setData] each frame.
 */
@SuppressLint("ViewConstructor")
internal class GlanceOverlayView(context: Context) : View(context) {

    var collapsed = false

    private var data = DoubleArray(SnapshotIndex.SIZE) { Double.NaN }
    private var opacityFactor = 0.85f

    private val density = resources.displayMetrics.density
    private fun dp(value: Float) = value * density

    private val backgroundPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply { color = Color.BLACK }
    private val titlePaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.parseColor("#4CD964")  // accent green
        textSize = dp(13f)
        typeface = Typeface.create(Typeface.MONOSPACE, Typeface.BOLD)
    }
    private val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        textSize = dp(13f)
        typeface = Typeface.MONOSPACE
    }

    private val padding = dp(10f)
    private val lineHeight = dp(18f)
    private val corner = dp(10f)
    private val panelWidth = dp(150f)
    private val handleSize = dp(40f)

    fun setData(values: DoubleArray) {
        data = values
        invalidate()
    }

    fun setOpacityFactor(factor: Float) {
        opacityFactor = factor
        invalidate()
    }

    fun toggleCollapsed() {
        collapsed = !collapsed
        requestLayout()
        invalidate()
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        if (collapsed) {
            setMeasuredDimension(handleSize.toInt(), handleSize.toInt())
        } else {
            val height = padding * 2 + lineHeight * (1 + visibleMetricCount())
            setMeasuredDimension(panelWidth.toInt(), height.toInt())
        }
    }

    override fun onDraw(canvas: Canvas) {
        backgroundPaint.alpha = (opacityFactor * 255).toInt().coerceIn(0, 255)
        val rect = RectF(0f, 0f, width.toFloat(), height.toFloat())
        canvas.drawRoundRect(rect, corner, corner, backgroundPaint)

        if (collapsed) {
            titlePaint.textSize = dp(18f)
            canvas.drawText("G", width / 2f - dp(6f), height / 2f + dp(6f), titlePaint)
            titlePaint.textSize = dp(13f)
            return
        }

        var y = padding + lineHeight - dp(4f)
        canvas.drawText("GlanceHUD", padding, y, titlePaint)

        for (line in metricLines()) {
            y += lineHeight
            textPaint.color = line.color
            canvas.drawText(line.text, padding, y, textPaint)
        }
    }

    /** One rendered metric line with its performance-based color. */
    private data class Line(val text: String, val color: Int)

    private fun fpsColor(value: Double): Int = when {
        value >= 50 -> COLOR_GOOD
        value >= 30 -> COLOR_WARN
        else -> COLOR_BAD
    }

    private fun cpuColor(value: Double): Int = when {
        value < 60 -> Color.WHITE
        value < 85 -> COLOR_WARN
        else -> COLOR_BAD
    }

    private fun valid(index: Int) = !data[index].isNaN()

    private fun visibleMetricCount(): Int {
        var count = 0
        if (valid(SnapshotIndex.FPS)) count++
        if (valid(SnapshotIndex.CPU)) count++
        if (valid(SnapshotIndex.RAM)) count++
        if (valid(SnapshotIndex.GPU)) count++
        if (valid(SnapshotIndex.NET_UP) || valid(SnapshotIndex.NET_DOWN)) count++
        return count.coerceAtLeast(1)  // Always leave room for a "waiting" line.
    }

    private fun metricLines(): List<Line> {
        val lines = mutableListOf<Line>()
        if (valid(SnapshotIndex.FPS)) {
            val fps = data[SnapshotIndex.FPS]
            lines += Line("FPS : ${fps.toInt()}", fpsColor(fps))
        }
        if (valid(SnapshotIndex.CPU)) {
            val cpu = data[SnapshotIndex.CPU]
            lines += Line("CPU : ${cpu.toInt()} %", cpuColor(cpu))
        }
        if (valid(SnapshotIndex.RAM)) {
            lines += Line("RAM : ${data[SnapshotIndex.RAM].toInt()} MB", Color.WHITE)
        }
        if (valid(SnapshotIndex.GPU)) {
            lines += Line("GPU : ${data[SnapshotIndex.GPU].toInt()} ms", Color.WHITE)
        }
        if (valid(SnapshotIndex.NET_UP) || valid(SnapshotIndex.NET_DOWN)) {
            val up = if (valid(SnapshotIndex.NET_UP)) data[SnapshotIndex.NET_UP].toInt() else 0
            val down = if (valid(SnapshotIndex.NET_DOWN)) data[SnapshotIndex.NET_DOWN].toInt() else 0
            lines += Line("NET : ↑$up ↓$down", Color.WHITE)
        }
        if (lines.isEmpty()) lines += Line("...", Color.WHITE)
        return lines
    }

    private companion object {
        val COLOR_GOOD = Color.parseColor("#4CD964")  // green
        val COLOR_WARN = Color.parseColor("#FF9500")  // orange
        val COLOR_BAD = Color.parseColor("#FF3B30")   // red
    }
}
