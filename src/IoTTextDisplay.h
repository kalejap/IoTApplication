/*
  IoTTextDisplay.h - Abstract interface for character (row/col) text displays.
  Copyright (c) 2024 Peter Kaleja.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <Arduino.h>

/**
 * @brief Abstract interface for a character (row × col) text display.
 *
 * Concrete subclasses wrap specific hardware (e.g. LiquidCrystal_I2C).
 * Optional capabilities (backlight, custom characters) are provided as
 * virtual no-ops so subclasses without those features still compile.
 */
class IoTTextDisplay
{
public:
    virtual ~IoTTextDisplay() = default;

    /**
     * @brief Initialise the display hardware. Call once in setup.
     */
    virtual void begin() = 0;

    /**
     * @brief Clear the entire display.
     */
    virtual void clear() = 0;

    /**
     * @brief Number of character columns.
     */
    virtual uint8_t cols() const = 0;

    /**
     * @brief Number of character rows.
     */
    virtual uint8_t rows() const = 0;

    /**
     * @brief Move the cursor to column col (0-based), row row (0-based).
     */
    virtual void setCursor(uint8_t col, uint8_t row) = 0;

    /**
     * @brief Print a null-terminated string from RAM.
     */
    virtual void print(const char* text) = 0;

    /**
     * @brief Print a flash (PROGMEM) string via F().
     */
    virtual void print(const __FlashStringHelper* text) = 0;

    /**
     * @brief Print a signed integer.
     */
    virtual void print(int value) = 0;

    /**
     * @brief Print a float with the given number of decimal places.
     */
    virtual void print(float value, uint8_t decimals = 1) = 0;

    /**
     * @brief Print an Arduino String (delegates to print(const char*)).
     */
    void print(const String& text) { print(text.c_str()); }

    /**
     * @brief Write text starting at column 0 of the given row, padding the
     *        remainder of the row with spaces so old content is erased without
     *        a full clear() call (avoids full-screen flicker).
     */
    void printLine(uint8_t row, const char* text)
    {
        setCursor(0, row);
        print(text);
        uint8_t len = 0;
        for (const char* p = text; *p; ++p) ++len;
        for (uint8_t i = len; i < cols(); ++i)
            print(" ");
    }

    // --- Optional capabilities — default no-ops ---

    /**
     * @brief Turn the backlight on (true) or off (false).
     */
    virtual void setBacklight(bool on) {}

    /**
     * @brief Define a custom character at slot index (0–7) from an 8-byte
     *        bitmap. Slots are preserved across clear().
     */
    virtual void createChar(uint8_t index, const uint8_t charmap[8]) {}

    /**
     * @brief Print the custom character stored at slot index.
     */
    virtual void writeChar(uint8_t index) {}
};
