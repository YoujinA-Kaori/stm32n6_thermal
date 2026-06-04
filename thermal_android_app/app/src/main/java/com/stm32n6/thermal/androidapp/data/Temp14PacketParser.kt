package com.stm32n6.thermal.androidapp.data

private const val SYNC_WORD = 0x55AA
private const val PACKET_TYPE_TEMP14 = 0x0001
private const val HEADER_SIZE_BYTES = 20
private const val MAX_BUFFER_BYTES = 512 * 1024

class Temp14PacketParser {
    private var buffer = ByteArray(0)

    fun reset() {
        buffer = ByteArray(0)
    }

    fun feed(chunk: ByteArray): List<Temp14Frame> {
        if (chunk.isEmpty()) {
            return emptyList()
        }

        appendChunk(chunk)
        val frames = mutableListOf<Temp14Frame>()

        while (true) {
            val syncIndex = findSyncIndex(buffer)
            if (syncIndex < 0) {
                buffer = if (buffer.size > 1) {
                    byteArrayOf(buffer.last())
                } else {
                    buffer
                }
                break
            }

            if (syncIndex > 0) {
                buffer = buffer.copyOfRange(syncIndex, buffer.size)
            }

            if (buffer.size < HEADER_SIZE_BYTES) {
                break
            }

            val syncWord = readU16Le(buffer, 0)
            if (syncWord != SYNC_WORD) {
                buffer = buffer.copyOfRange(1, buffer.size)
                continue
            }

            val packetType = readU16Le(buffer, 2)
            val frameCounter = readU32Le(buffer, 4)
            val frameWidth = readU16Le(buffer, 8)
            val frameHeight = readU16Le(buffer, 10)
            val payloadBytes = readU16Le(buffer, 12)
            val centerTemp14 = readU16Le(buffer, 14)

            val expectedPayloadBytes = frameWidth * frameHeight * 2
            val packetBytes = HEADER_SIZE_BYTES + payloadBytes

            if (
                packetType != PACKET_TYPE_TEMP14 ||
                frameWidth <= 0 ||
                frameHeight <= 0 ||
                payloadBytes != expectedPayloadBytes
            ) {
                buffer = buffer.copyOfRange(1, buffer.size)
                continue
            }

            if (buffer.size < packetBytes) {
                break
            }

            val payload = buffer.copyOfRange(HEADER_SIZE_BYTES, packetBytes)
            val values = IntArray(frameWidth * frameHeight)
            for (index in values.indices) {
                values[index] = readU16Le(payload, index * 2)
            }

            frames += Temp14Frame.fromPacket(
                frameCounter = frameCounter,
                width = frameWidth,
                height = frameHeight,
                centerTemp14FromHeader = centerTemp14,
                values = values
            )

            buffer = buffer.copyOfRange(packetBytes, buffer.size)
        }

        return frames
    }

    private fun appendChunk(chunk: ByteArray) {
        val newSize = buffer.size + chunk.size
        if (newSize <= MAX_BUFFER_BYTES) {
            buffer += chunk
            return
        }

        val keepBytes = (MAX_BUFFER_BYTES - chunk.size).coerceAtLeast(0)
        buffer = if (keepBytes > 0 && buffer.isNotEmpty()) {
            buffer.copyOfRange(buffer.size - keepBytes, buffer.size) + chunk
        } else {
            chunk.copyOfLast(MAX_BUFFER_BYTES.coerceAtMost(chunk.size))
        }
    }

    private fun findSyncIndex(data: ByteArray): Int {
        for (index in 0 until data.size - 1) {
            if ((data[index].toInt() and 0xFF) == 0xAA && (data[index + 1].toInt() and 0xFF) == 0x55) {
                return index
            }
        }
        return -1
    }

    private fun readU16Le(data: ByteArray, offset: Int): Int {
        return (data[offset].toInt() and 0xFF) or
            ((data[offset + 1].toInt() and 0xFF) shl 8)
    }

    private fun readU32Le(data: ByteArray, offset: Int): Long {
        return (data[offset].toLong() and 0xFFL) or
            ((data[offset + 1].toLong() and 0xFFL) shl 8) or
            ((data[offset + 2].toLong() and 0xFFL) shl 16) or
            ((data[offset + 3].toLong() and 0xFFL) shl 24)
    }

    private fun ByteArray.copyOfLast(count: Int): ByteArray {
        if (count <= 0) {
            return ByteArray(0)
        }
        return copyOfRange(size - count, size)
    }
}
