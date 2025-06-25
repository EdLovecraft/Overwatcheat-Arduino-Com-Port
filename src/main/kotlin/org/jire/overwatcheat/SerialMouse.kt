package org.jire.overwatcheat

import com.fazecast.jSerialComm.SerialPort
import org.jire.overwatcheat.settings.Settings

object SerialMouse {
    private var port: SerialPort? = null

    private fun open() {
        if (port == null || !(port?.isOpen ?: false)) {
            port = SerialPort.getCommPort(Settings.comPort)
            port?.setBaudRate(115200)
            port?.setNumDataBits(8)
            port?.setNumStopBits(SerialPort.ONE_STOP_BIT)
            port?.setParity(SerialPort.NO_PARITY)
            port?.openPort()
        }
    }

    fun move(x: Int, y: Int) {
        open()
        val cmd = "M${x},${y}\n"
        port?.outputStream?.write(cmd.toByteArray())
        port?.outputStream?.flush()
    }

    fun click() {
        open()
        val cmd = "C\n"
        port?.outputStream?.write(cmd.toByteArray())
        port?.outputStream?.flush()
    }
} 