package com.stm32n6.thermal.androidapp.ui

import android.graphics.Color
import kotlin.math.roundToInt

enum class PseudoPalette(val label: String, private val table: IntArray) {
    THERMAL(
        "热力",
        buildGradient(
            listOf(
                0.00f to intArrayOf(0, 0, 0),
                0.16f to intArrayOf(0, 24, 112),
                0.34f to intArrayOf(0, 100, 255),
                0.50f to intArrayOf(0, 210, 255),
                0.66f to intArrayOf(80, 255, 120),
                0.82f to intArrayOf(255, 225, 54),
                0.92f to intArrayOf(255, 96, 0),
                1.00f to intArrayOf(255, 255, 255)
            )
        )
    ),
    RAINBOW(
        "彩虹",
        buildGradient(
            listOf(
                0.00f to intArrayOf(0, 0, 120),
                0.20f to intArrayOf(0, 96, 255),
                0.40f to intArrayOf(0, 224, 255),
                0.58f to intArrayOf(0, 255, 96),
                0.75f to intArrayOf(255, 240, 0),
                0.88f to intArrayOf(255, 128, 0),
                1.00f to intArrayOf(255, 0, 0)
            )
        )
    ),
    IRON(
        "铁红",
        buildGradient(
            listOf(
                0.00f to intArrayOf(0, 0, 0),
                0.25f to intArrayOf(64, 0, 64),
                0.50f to intArrayOf(160, 32, 0),
                0.75f to intArrayOf(255, 128, 0),
                0.90f to intArrayOf(255, 220, 110),
                1.00f to intArrayOf(255, 255, 255)
            )
        )
    ),
    GRAY(
        "灰度",
        buildGradient(
            listOf(
                0.00f to intArrayOf(0, 0, 0),
                1.00f to intArrayOf(255, 255, 255)
            )
        )
    );

    fun colorFor(normalized: Float): Int {
        val clamped = normalized.coerceIn(0.0f, 1.0f)
        val index = (clamped * 255.0f).roundToInt().coerceIn(0, 255)
        return table[index]
    }
}

private fun buildGradient(stops: List<Pair<Float, IntArray>>): IntArray {
    val colors = IntArray(256)
    for (index in 0..255) {
        val t = index / 255.0f
        var left = stops.first()
        var right = stops.last()

        for (stopIndex in 0 until stops.size - 1) {
            val candidateLeft = stops[stopIndex]
            val candidateRight = stops[stopIndex + 1]
            if (t >= candidateLeft.first && t <= candidateRight.first) {
                left = candidateLeft
                right = candidateRight
                break
            }
        }

        val span = (right.first - left.first).takeIf { it > 0.0f } ?: 1.0f
        val local = ((t - left.first) / span).coerceIn(0.0f, 1.0f)
        val r = lerpColorChannel(left.second[0], right.second[0], local)
        val g = lerpColorChannel(left.second[1], right.second[1], local)
        val b = lerpColorChannel(left.second[2], right.second[2], local)
        colors[index] = Color.rgb(r, g, b)
    }
    return colors
}

private fun lerpColorChannel(start: Int, end: Int, t: Float): Int {
    return (start + (end - start) * t).roundToInt().coerceIn(0, 255)
}
