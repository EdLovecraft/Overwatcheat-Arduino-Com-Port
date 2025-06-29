/*
 * Free, open-source undetected color cheat for Overwatch!
 * Copyright (C) 2017  Thomas G. Nappo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package org.jire.overwatcheat

import java.awt.Dimension
import java.awt.Toolkit
import org.jire.overwatcheat.nativelib.User32
import org.jire.overwatcheat.nativelib.User32Constants

object Screen {

    // Logical screen dimensions (DPI-aware, what Java reports)
    private val LOGICAL_DIMENSION: Dimension = Toolkit.getDefaultToolkit().screenSize
    val LOGICAL_WIDTH = LOGICAL_DIMENSION.width
    val LOGICAL_HEIGHT = LOGICAL_DIMENSION.height

    // Physical screen dimensions (actual pixels, what FFmpeg captures)
    val PHYSICAL_WIDTH = User32.GetSystemMetrics(User32Constants.SM_CXSCREEN)
    val PHYSICAL_HEIGHT = User32.GetSystemMetrics(User32Constants.SM_CYSCREEN)

    // DPI scaling factor
    val DPI_SCALE_X = PHYSICAL_WIDTH.toFloat() / LOGICAL_WIDTH
    val DPI_SCALE_Y = PHYSICAL_HEIGHT.toFloat() / LOGICAL_HEIGHT

    // For backward compatibility, use physical dimensions as default
    val WIDTH = PHYSICAL_WIDTH
    val HEIGHT = PHYSICAL_HEIGHT

    const val OVERLAY_OFFSET = 1
    val OVERLAY_WIDTH = WIDTH - OVERLAY_OFFSET
    val OVERLAY_HEIGHT = HEIGHT - OVERLAY_OFFSET

    /**
     * Convert logical coordinates to physical coordinates
     */
    fun logicalToPhysical(logicalX: Int, logicalY: Int): Pair<Int, Int> {
        return Pair(
            (logicalX * DPI_SCALE_X).toInt(),
            (logicalY * DPI_SCALE_Y).toInt()
        )
    }

    /**
     * Convert physical coordinates to logical coordinates
     */
    fun physicalToLogical(physicalX: Int, physicalY: Int): Pair<Int, Int> {
        return Pair(
            (physicalX / DPI_SCALE_X).toInt(),
            (physicalY / DPI_SCALE_Y).toInt()
        )
    }

    /**
     * Print DPI scaling information for debugging
     */
    fun printDpiInfo() {
        println("=== DPI Scaling Information ===")
        println("Logical dimensions: ${LOGICAL_WIDTH}x${LOGICAL_HEIGHT}")
        println("Physical dimensions: ${PHYSICAL_WIDTH}x${PHYSICAL_HEIGHT}")
        println("DPI Scale X: $DPI_SCALE_X")
        println("DPI Scale Y: $DPI_SCALE_Y")
        println("===============================")
    }

}
