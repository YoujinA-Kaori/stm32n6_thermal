package com.stm32n6.thermal.androidapp.data

import kotlin.math.roundToInt

data class Temp14Frame(
    val frameCounter: Long,
    val width: Int,
    val height: Int,
    val centerTemp14: Int,
    val minTemp14: Int,
    val maxTemp14: Int,
    val minX: Int,
    val minY: Int,
    val maxX: Int,
    val maxY: Int,
    val values: IntArray
) {
    companion object {
        fun fromPacket(
            frameCounter: Long,
            width: Int,
            height: Int,
            centerTemp14FromHeader: Int,
            values: IntArray
        ): Temp14Frame {
            var minTemp14 = Int.MAX_VALUE
            var maxTemp14 = Int.MIN_VALUE
            var minX = 0
            var minY = 0
            var maxX = 0
            var maxY = 0

            for (index in values.indices) {
                val value = values[index]
                val x = index % width
                val y = index / width

                if (value < minTemp14) {
                    minTemp14 = value
                    minX = x
                    minY = y
                }

                if (value > maxTemp14) {
                    maxTemp14 = value
                    maxX = x
                    maxY = y
                }
            }

            val centerIndex = (height / 2) * width + (width / 2)
            val centerTemp14 = if (centerIndex in values.indices) {
                values[centerIndex]
            } else {
                centerTemp14FromHeader
            }

            return Temp14Frame(
                frameCounter = frameCounter,
                width = width,
                height = height,
                centerTemp14 = centerTemp14,
                minTemp14 = minTemp14,
                maxTemp14 = maxTemp14,
                minX = minX,
                minY = minY,
                maxX = maxX,
                maxY = maxY,
                values = values
            )
        }
    }
}

fun temp14ToCelsius(temp14: Int): Float {
    return temp14 / 16.0f - 273.15f
}

fun celsiusToTemp14(tempC: Float): Int {
    return ((tempC + 273.15f) * 16.0f).roundToInt()
}
