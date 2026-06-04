package com.stm32n6.thermal.androidapp.usb

import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbDeviceConnection
import android.hardware.usb.UsbManager
import android.os.Build
import com.hoho.android.usbserial.driver.UsbSerialDriver
import com.hoho.android.usbserial.driver.UsbSerialPort
import com.hoho.android.usbserial.driver.UsbSerialProber
import com.stm32n6.thermal.androidapp.R
import com.stm32n6.thermal.androidapp.data.Temp14Frame
import com.stm32n6.thermal.androidapp.data.Temp14PacketParser
import java.util.Locale
import java.util.concurrent.atomic.AtomicBoolean

class UsbSerialController(
    private val context: Context,
    private val listener: Listener
) {
    interface Listener {
        fun onDeviceListChanged()
        fun onStatusChanged(status: String)
        fun onFrameReceived(frame: Temp14Frame)
        fun onError(message: String, throwable: Throwable? = null)
    }

    private val usbManager = context.getSystemService(Context.USB_SERVICE) as UsbManager
    private val parser = Temp14PacketParser()
    private val stopRequested = AtomicBoolean(false)
    private val permissionIntent = PendingIntent.getBroadcast(
        context,
        0,
        Intent(ACTION_USB_PERMISSION).setPackage(context.packageName),
        PendingIntent.FLAG_IMMUTABLE
    )

    private var receiversRegistered = false
    private var pendingDriver: UsbSerialDriver? = null
    private var currentDriver: UsbSerialDriver? = null
    private var currentConnection: UsbDeviceConnection? = null
    private var currentPort: UsbSerialPort? = null
    private var readerThread: Thread? = null

    private val usbReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            when (intent.action) {
                ACTION_USB_PERMISSION -> handlePermissionResult(intent)
                UsbManager.ACTION_USB_DEVICE_ATTACHED -> listener.onDeviceListChanged()
                UsbManager.ACTION_USB_DEVICE_DETACHED -> handleDetached(intent)
            }
        }
    }

    fun register() {
        if (receiversRegistered) {
            return
        }

        val filter = IntentFilter().apply {
            addAction(ACTION_USB_PERMISSION)
            addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED)
            addAction(UsbManager.ACTION_USB_DEVICE_DETACHED)
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            context.registerReceiver(usbReceiver, filter, Context.RECEIVER_NOT_EXPORTED)
        } else {
            context.registerReceiver(usbReceiver, filter)
        }
        receiversRegistered = true
    }

    fun unregister() {
        if (!receiversRegistered) {
            return
        }
        context.unregisterReceiver(usbReceiver)
        receiversRegistered = false
    }

    fun availableDrivers(): List<UsbSerialDriver> {
        return UsbSerialProber.getDefaultProber().findAllDrivers(usbManager)
    }

    fun describeDriver(driver: UsbSerialDriver): String {
        val device = driver.device
        val productName = device.productName ?: "USB 串口"
        return String.format(
            Locale.US,
            "%s (VID %04X / PID %04X)",
            productName,
            device.vendorId,
            device.productId
        )
    }

    fun isConnected(): Boolean {
        return currentPort != null
    }

    fun connect(driver: UsbSerialDriver) {
        if (isConnected()) {
            close()
        }

        currentDriver = driver
        if (!usbManager.hasPermission(driver.device)) {
            pendingDriver = driver
            listener.onStatusChanged(context.getString(R.string.status_request_usb_permission))
            usbManager.requestPermission(driver.device, permissionIntent)
            return
        }

        openDriver(driver)
    }

    fun close() {
        stopRequested.set(true)
        val rThread = readerThread
        if (rThread != null && Thread.currentThread() !== rThread) {
            rThread.join(500)
        }
        readerThread = null

        try {
            currentPort?.close()
        } catch (_: Throwable) {
        }
        currentPort = null

        try {
            currentConnection?.close()
        } catch (_: Throwable) {
        }
        currentConnection = null
        currentDriver = null
        parser.reset()

        listener.onStatusChanged(context.getString(R.string.status_disconnected))
    }

    private fun handlePermissionResult(intent: Intent) {
        val granted = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)
        val driver = pendingDriver
        pendingDriver = null

        if (!granted || driver == null) {
            val message = context.getString(R.string.status_usb_permission_denied)
            listener.onError(message)
            listener.onStatusChanged(message)
            return
        }

        openDriver(driver)
    }

    private fun handleDetached(intent: Intent) {
        val detached = intent.parcelableExtraCompat<UsbDevice>(UsbManager.EXTRA_DEVICE) ?: return
        if (currentDriver?.device?.deviceId == detached.deviceId) {
            close()
        }
        listener.onDeviceListChanged()
    }

    private fun openDriver(driver: UsbSerialDriver) {
        try {
            val connection = usbManager.openDevice(driver.device)
            if (connection == null) {
                listener.onError(context.getString(R.string.error_open_usb_device))
                return
            }

            val port = driver.ports.firstOrNull()
            if (port == null) {
                connection.close()
                listener.onError(context.getString(R.string.error_no_serial_port))
                return
            }

            port.open(connection)
            port.setParameters(921600, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE)
            try {
                port.setDTR(true)
                port.setRTS(true)
            } catch (_: Throwable) {
            }

            currentConnection = connection
            currentPort = port
            currentDriver = driver
            listener.onStatusChanged(context.getString(R.string.status_connected, describeDriver(driver)))
            startReader(port)
        } catch (throwable: Throwable) {
            listener.onError(context.getString(R.string.status_connect_failed), throwable)
            close()
        }
    }

    private fun startReader(port: UsbSerialPort) {
        stopRequested.set(false)
        parser.reset()

        readerThread = Thread {
            val readBuffer = ByteArray(8192)
            while (!stopRequested.get()) {
                try {
                    val len = port.read(readBuffer, 100)
                    if (len > 0) {
                        val copy = ByteArray(len)
                        System.arraycopy(readBuffer, 0, copy, 0, len)
                        val frames = parser.feed(copy)
                        for (frame in frames) {
                            listener.onFrameReceived(frame)
                        }
                    }
                } catch (throwable: Throwable) {
                    if (!stopRequested.get()) {
                        listener.onError(context.getString(R.string.status_serial_read_failed), throwable)
                        close()
                    }
                    return@Thread
                }
            }
        }.apply {
            isDaemon = true
            name = "thermal-usb-reader"
            priority = Thread.MAX_PRIORITY
            start()
        }
    }

    private inline fun <reified T> Intent.parcelableExtraCompat(name: String): T? {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            getParcelableExtra(name, T::class.java)
        } else {
            @Suppress("DEPRECATION")
            getParcelableExtra(name)
        }
    }

    companion object {
        private const val ACTION_USB_PERMISSION = "com.stm32n6.thermal.androidapp.USB_PERMISSION"
    }
}
