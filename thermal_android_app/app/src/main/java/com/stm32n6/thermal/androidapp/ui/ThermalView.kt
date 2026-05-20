package com.stm32n6.thermal.androidapp.ui

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RectF
import android.util.AttributeSet
import android.view.View
import com.stm32n6.thermal.androidapp.R
import com.stm32n6.thermal.androidapp.data.Temp14Frame
import com.stm32n6.thermal.androidapp.data.celsiusToTemp14
import com.stm32n6.thermal.androidapp.data.temp14ToCelsius
import java.util.Locale
import kotlin.math.max
import kotlin.math.min

class ThermalView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : View(context, attrs) {

    private var currentFrame: Temp14Frame? = null
    private var frameBitmap: Bitmap? = null
    private var palette: PseudoPalette = PseudoPalette.THERMAL
    private var autoRange: Boolean = true
    private var lowRangeC: Float = 20.0f
    private var highRangeC: Float = 50.0f

    private val backgroundPaint = Paint().apply { color = Color.BLACK }
    private val bitmapPaint = Paint(Paint.FILTER_BITMAP_FLAG)
    private val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        textSize = 30.0f
    }
    private val badgePaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val badgeStrokePaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.STROKE
        strokeWidth = 2.0f
    }
    private val markerPaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val crossPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        strokeWidth = 3.0f
    }
    private val placeholderPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.rgb(185, 198, 210)
        textSize = 38.0f
    }

    fun setPalette(value: PseudoPalette) {
        palette = value
        rebuildBitmap()
        invalidate()
    }

    fun setRange(auto: Boolean, lowC: Float, highC: Float) {
        autoRange = auto
        lowRangeC = lowC
        highRangeC = max(lowC + 1.0f, highC)
        rebuildBitmap()
        invalidate()
    }

    fun updateFrame(frame: Temp14Frame) {
        currentFrame = frame
        rebuildBitmap()
        invalidate()
    }

    fun captureBitmap(): Bitmap {
        val output = Bitmap.createBitmap(width.coerceAtLeast(1), height.coerceAtLeast(1), Bitmap.Config.ARGB_8888)
        val canvas = Canvas(output)
        draw(canvas)
        return output
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        canvas.drawRect(0.0f, 0.0f, width.toFloat(), height.toFloat(), backgroundPaint)

        val frame = currentFrame
        val bitmap = frameBitmap
        if (frame == null || bitmap == null) {
            drawPlaceholder(canvas)
            return
        }

        val destRect = computeDestRect(frame.width, frame.height)
        canvas.drawBitmap(bitmap, null, destRect, bitmapPaint)

        val centerX = destRect.left + destRect.width() * 0.5f
        val centerY = destRect.top + destRect.height() * 0.5f
        canvas.drawLine(centerX - 18.0f, centerY, centerX + 18.0f, centerY, crossPaint)
        canvas.drawLine(centerX, centerY - 18.0f, centerX, centerY + 18.0f, crossPaint)

        drawMarker(canvas, destRect, frame.maxX, frame.maxY, "最高", Color.rgb(255, 122, 122))
        drawMarker(canvas, destRect, frame.minX, frame.minY, "最低", Color.rgb(105, 220, 255))

        drawBadge(canvas, 18.0f, 18.0f, "中心 ${formatTemp(frame.centerTemp14)}", Color.WHITE)
        drawBadge(
            canvas,
            width - 238.0f,
            18.0f,
            "最高 ${formatTemp(frame.maxTemp14)}",
            Color.rgb(255, 139, 66)
        )
        drawBadge(
            canvas,
            18.0f,
            height - 62.0f,
            "最低 ${formatTemp(frame.minTemp14)}",
            Color.rgb(76, 200, 255)
        )
        drawBadge(
            canvas,
            width - 190.0f,
            height - 62.0f,
            palette.label,
            Color.rgb(255, 180, 84)
        )
    }

    private fun drawPlaceholder(canvas: Canvas) {
        val text = context.getString(R.string.placeholder_connect_device)
        val bounds = android.graphics.Rect()
        placeholderPaint.getTextBounds(text, 0, text.length, bounds)
        canvas.drawText(
            text,
            (width - bounds.width()) * 0.5f,
            (height + bounds.height()) * 0.5f,
            placeholderPaint
        )
    }

    private fun rebuildBitmap() {
        val frame = currentFrame ?: return
        val bitmap = if (frameBitmap?.width == frame.width && frameBitmap?.height == frame.height) {
            frameBitmap!!
        } else {
            Bitmap.createBitmap(frame.width, frame.height, Bitmap.Config.ARGB_8888).also {
                frameBitmap = it
            }
        }

        val activeLowC = if (autoRange) temp14ToCelsius(frame.minTemp14) else lowRangeC
        val activeHighC = if (autoRange) temp14ToCelsius(frame.maxTemp14) else highRangeC
        val lowTemp14 = celsiusToTemp14(activeLowC)
        val highTemp14 = celsiusToTemp14(activeHighC)
        val rangeTemp14 = max(1, highTemp14 - lowTemp14)

        val pixels = IntArray(frame.values.size)
        for (index in frame.values.indices) {
            val normalized = (frame.values[index] - lowTemp14).toFloat() / rangeTemp14.toFloat()
            pixels[index] = palette.colorFor(normalized)
        }

        bitmap.setPixels(pixels, 0, frame.width, 0, 0, frame.width, frame.height)
    }

    private fun computeDestRect(frameWidth: Int, frameHeight: Int): RectF {
        val availableWidth = width.toFloat()
        val availableHeight = height.toFloat()
        val scale = min(availableWidth / frameWidth.toFloat(), availableHeight / frameHeight.toFloat())
        val drawWidth = frameWidth * scale
        val drawHeight = frameHeight * scale
        val left = (availableWidth - drawWidth) * 0.5f
        val top = (availableHeight - drawHeight) * 0.5f
        return RectF(left, top, left + drawWidth, top + drawHeight)
    }

    private fun drawMarker(
        canvas: Canvas,
        dest: RectF,
        frameX: Int,
        frameY: Int,
        label: String,
        color: Int
    ) {
        val frame = currentFrame ?: return
        val x = dest.left + ((frameX + 0.5f) / frame.width.toFloat()) * dest.width()
        val y = dest.top + ((frameY + 0.5f) / frame.height.toFloat()) * dest.height()

        markerPaint.color = color
        canvas.drawCircle(x, y, 8.0f, markerPaint)
        drawBadge(canvas, x + 12.0f, y - 18.0f, label, color)
    }

    private fun drawBadge(canvas: Canvas, left: Float, top: Float, text: String, accent: Int) {
        val textWidth = textPaint.measureText(text)
        val badgeRect = RectF(left, top, left + textWidth + 26.0f, top + 38.0f)

        badgePaint.color = Color.argb(190, 0, 0, 0)
        badgeStrokePaint.color = accent
        canvas.drawRoundRect(badgeRect, 10.0f, 10.0f, badgePaint)
        canvas.drawRoundRect(badgeRect, 10.0f, 10.0f, badgeStrokePaint)
        canvas.drawText(text, badgeRect.left + 13.0f, badgeRect.bottom - 11.0f, textPaint)
    }

    private fun formatTemp(temp14: Int): String {
        return String.format(Locale.US, "%.2f °C", temp14ToCelsius(temp14))
    }
}
