package com.stm32n6.thermal.androidapp

import android.content.ContentValues
import android.graphics.Bitmap
import android.os.Bundle
import android.os.Environment
import android.provider.MediaStore
import android.view.View
import android.widget.ArrayAdapter
import android.widget.SeekBar
import android.widget.Spinner
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.button.MaterialButton
import com.google.android.material.materialswitch.MaterialSwitch
import com.google.android.material.snackbar.Snackbar
import com.hoho.android.usbserial.driver.UsbSerialDriver
import com.stm32n6.thermal.androidapp.data.Temp14Frame
import com.stm32n6.thermal.androidapp.data.temp14ToCelsius
import com.stm32n6.thermal.androidapp.ui.PseudoPalette
import com.stm32n6.thermal.androidapp.ui.ThermalView
import com.stm32n6.thermal.androidapp.usb.UsbSerialController
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class MainActivity : AppCompatActivity(), UsbSerialController.Listener {
    private lateinit var controller: UsbSerialController
    private lateinit var thermalView: ThermalView
    private lateinit var statusText: TextView
    private lateinit var deviceSpinner: Spinner
    private lateinit var paletteSpinner: Spinner
    private lateinit var autoRangeSwitch: MaterialSwitch
    private lateinit var lowRangeSeek: SeekBar
    private lateinit var highRangeSeek: SeekBar
    private lateinit var lowRangeLabel: TextView
    private lateinit var highRangeLabel: TextView
    private lateinit var frameText: TextView
    private lateinit var centerText: TextView
    private lateinit var maxText: TextView
    private lateinit var minText: TextView
    private lateinit var pauseButton: MaterialButton

    private var drivers: List<UsbSerialDriver> = emptyList()
    private var paused = false
    private var latestFrame: Temp14Frame? = null
    private var autoConnectArmed = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        controller = UsbSerialController(this, this)
        bindViews()
        setupControls()
        refreshDeviceList()
        applyDisplayOptions()
    }

    override fun onStart() {
        super.onStart()
        controller.register()
        autoConnectArmed = true
        refreshDeviceList()
    }

    override fun onStop() {
        controller.unregister()
        super.onStop()
    }

    override fun onDestroy() {
        controller.close()
        super.onDestroy()
    }

    override fun onDeviceListChanged() {
        runOnUiThread {
            autoConnectArmed = true
            refreshDeviceList()
        }
    }

    override fun onStatusChanged(status: String) {
        runOnUiThread { statusText.text = status }
    }

    override fun onFrameReceived(frame: Temp14Frame) {
        latestFrame = frame
        if (paused) {
            return
        }
        runOnUiThread { renderFrame(frame) }
    }

    override fun onError(message: String, throwable: Throwable?) {
        runOnUiThread {
            statusText.text = message
            Snackbar.make(thermalView, message, Snackbar.LENGTH_LONG).show()
        }
    }

    private fun bindViews() {
        thermalView = findViewById(R.id.thermalView)
        statusText = findViewById(R.id.statusText)
        deviceSpinner = findViewById(R.id.deviceSpinner)
        paletteSpinner = findViewById(R.id.paletteSpinner)
        autoRangeSwitch = findViewById(R.id.autoRangeSwitch)
        lowRangeSeek = findViewById(R.id.lowRangeSeek)
        highRangeSeek = findViewById(R.id.highRangeSeek)
        lowRangeLabel = findViewById(R.id.lowRangeLabel)
        highRangeLabel = findViewById(R.id.highRangeLabel)
        frameText = findViewById(R.id.frameText)
        centerText = findViewById(R.id.centerText)
        maxText = findViewById(R.id.maxText)
        minText = findViewById(R.id.minText)
        pauseButton = findViewById(R.id.pauseButton)
    }

    private fun setupControls() {
        val paletteNames = PseudoPalette.values().map { it.label }
        paletteSpinner.adapter = ArrayAdapter(
            this,
            android.R.layout.simple_spinner_dropdown_item,
            paletteNames
        )

        lowRangeSeek.min = -20
        lowRangeSeek.max = 120
        lowRangeSeek.progress = 20

        highRangeSeek.min = -20
        highRangeSeek.max = 120
        highRangeSeek.progress = 50

        findViewById<MaterialButton>(R.id.refreshButton).setOnClickListener {
            autoConnectArmed = true
            refreshDeviceList()
        }

        findViewById<MaterialButton>(R.id.connectButton).setOnClickListener {
            val driver = selectedDriver()
            if (driver == null) {
                showMessage(getString(R.string.hint_no_device))
            } else {
                autoConnectArmed = false
                controller.connect(driver)
            }
        }

        findViewById<MaterialButton>(R.id.disconnectButton).setOnClickListener {
            autoConnectArmed = false
            controller.close()
        }

        pauseButton.setOnClickListener {
            paused = !paused
            pauseButton.setText(if (paused) R.string.label_resume else R.string.label_pause)
            latestFrame?.takeIf { !paused }?.let(::renderFrame)
        }

        findViewById<MaterialButton>(R.id.snapshotButton).setOnClickListener {
            saveSnapshot()
        }

        paletteSpinner.onItemSelectedListener = SimpleItemSelectedListener {
            applyDisplayOptions()
        }

        autoRangeSwitch.setOnCheckedChangeListener { _, _ ->
            updateRangeControls()
            applyDisplayOptions()
        }

        lowRangeSeek.setOnSeekBarChangeListener(simpleSeekChangeListener {
            keepValidRange(lowChanged = true)
            applyDisplayOptions()
        })

        highRangeSeek.setOnSeekBarChangeListener(simpleSeekChangeListener {
            keepValidRange(lowChanged = false)
            applyDisplayOptions()
        })

        frameText.text = getString(R.string.label_frame_value, getString(R.string.label_frame), 0, 0, 0)
        centerText.text = getString(R.string.label_temp_value, getString(R.string.label_center), "--")
        maxText.text = getString(R.string.label_temp_point_value, getString(R.string.label_max), "--", 0, 0)
        minText.text = getString(R.string.label_temp_point_value, getString(R.string.label_min), "--", 0, 0)
    }

    private fun refreshDeviceList() {
        drivers = controller.availableDrivers()
        val labels = if (drivers.isEmpty()) {
            listOf(getString(R.string.hint_no_device))
        } else {
            drivers.map { controller.describeDriver(it) }
        }

        val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_dropdown_item, labels)
        deviceSpinner.adapter = adapter
        if (drivers.isNotEmpty()) {
            deviceSpinner.setSelection(0, false)
        }
        attemptAutoConnectIfNeeded()
    }

    private fun selectedDriver(): UsbSerialDriver? {
        val index = deviceSpinner.selectedItemPosition
        return drivers.getOrNull(index)
    }

    private fun selectedPalette(): PseudoPalette {
        return PseudoPalette.values().getOrElse(paletteSpinner.selectedItemPosition) { PseudoPalette.THERMAL }
    }

    private fun keepValidRange(lowChanged: Boolean) {
        if (highRangeSeek.progress <= lowRangeSeek.progress) {
            if (lowChanged) {
                highRangeSeek.progress = (lowRangeSeek.progress + 1).coerceAtMost(highRangeSeek.max)
            } else {
                lowRangeSeek.progress = (highRangeSeek.progress - 1).coerceAtLeast(lowRangeSeek.min)
            }
        }
        updateRangeControls()
    }

    private fun updateRangeControls() {
        val autoRange = autoRangeSwitch.isChecked
        lowRangeSeek.isEnabled = !autoRange
        highRangeSeek.isEnabled = !autoRange
        lowRangeLabel.text = getString(R.string.label_range_unit, getString(R.string.label_low_range), lowRangeSeek.progress)
        highRangeLabel.text = getString(R.string.label_range_unit, getString(R.string.label_high_range), highRangeSeek.progress)
    }

    private fun applyDisplayOptions() {
        updateRangeControls()
        thermalView.setPalette(selectedPalette())
        thermalView.setRange(
            auto = autoRangeSwitch.isChecked,
            lowC = lowRangeSeek.progress.toFloat(),
            highC = highRangeSeek.progress.toFloat()
        )
        latestFrame?.let { if (!paused) renderFrame(it) }
    }

    private fun renderFrame(frame: Temp14Frame) {
        thermalView.updateFrame(frame)
        frameText.text = getString(R.string.label_frame_value, getString(R.string.label_frame), frame.frameCounter, frame.width, frame.height)
        centerText.text = getString(R.string.label_temp_value, getString(R.string.label_center), formatTemp(frame.centerTemp14))
        maxText.text = getString(R.string.label_temp_point_value, getString(R.string.label_max), formatTemp(frame.maxTemp14), frame.maxX, frame.maxY)
        minText.text = getString(R.string.label_temp_point_value, getString(R.string.label_min), formatTemp(frame.minTemp14), frame.minX, frame.minY)
    }

    private fun saveSnapshot() {
        val bitmap = thermalView.captureBitmap()
        val fileName = "thermal_${SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())}.png"
        val contentValues = ContentValues().apply {
            put(MediaStore.Images.Media.DISPLAY_NAME, fileName)
            put(MediaStore.Images.Media.MIME_TYPE, "image/png")
            put(MediaStore.Images.Media.RELATIVE_PATH, Environment.DIRECTORY_PICTURES + "/ThermalSerialViewer")
        }

        try {
            val uri = contentResolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, contentValues)
            if (uri == null) {
                showMessage(getString(R.string.error_snapshot_entry))
                return
            }

            val outputStream = contentResolver.openOutputStream(uri)
            if (outputStream == null) {
                showMessage(getString(R.string.error_snapshot_stream))
                return
            }

            outputStream.use { stream ->
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream)
            }
            showMessage(getString(R.string.message_snapshot_saved, fileName))
        } catch (throwable: Throwable) {
            showMessage(getString(R.string.message_snapshot_failed, throwable.message ?: "未知错误"))
        }
    }

    private fun formatTemp(temp14: Int): String {
        return String.format(Locale.US, "%.2f °C", temp14ToCelsius(temp14))
    }

    private fun showMessage(message: String) {
        Snackbar.make(thermalView, message, Snackbar.LENGTH_LONG).show()
    }

    private fun attemptAutoConnectIfNeeded() {
        if (!autoConnectArmed || controller.isConnected() || drivers.isEmpty()) {
            return
        }

        autoConnectArmed = false
        statusText.text = getString(R.string.status_auto_connecting)
        controller.connect(drivers.first())
    }

    private fun simpleSeekChangeListener(onChanged: () -> Unit): SeekBar.OnSeekBarChangeListener {
        return object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                onChanged()
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }
        }
    }

    private class SimpleItemSelectedListener(
        private val onSelected: () -> Unit
    ) : android.widget.AdapterView.OnItemSelectedListener {
        override fun onItemSelected(
            parent: android.widget.AdapterView<*>?,
            view: View?,
            position: Int,
            id: Long
        ) {
            onSelected()
        }

        override fun onNothingSelected(parent: android.widget.AdapterView<*>?) {
        }
    }
}
