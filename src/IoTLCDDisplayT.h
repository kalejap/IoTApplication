/*
  IoTLCDDisplayT.h - Generic template adapter wrapping any compatible I2C
  character LCD driver through the IoTTextDisplay interface.
  Copyright (c) 2026 Peter Kaleja.  All right reserved.

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
#include "IoTTextDisplay.h"
#include "LCDLanguageSupport.h"

#ifdef _IOT_REAL_TIME
#include <TimeLib.h>
#endif

/**
 * @brief Default driver traits.
 *
 * Assumes the driver exposes:
 *   - A constructor matching (uint8_t address, uint8_t cols, uint8_t rows)
 *   - begin(uint8_t cols, uint8_t rows) for hardware initialisation
 *   - No backlight control
 *
 * Specialise this struct for drivers that use a different init sequence
 * (e.g. LiquidCrystal_I2C, which uses init() + backlight()).
 */
template<typename TDriver>
struct IoTLCDDriverTraits
{
    static void begin(TDriver& lcd, uint8_t cols, uint8_t rows)
    {
        lcd.begin(cols, rows);
    }

    static void setBacklight(TDriver& /*lcd*/, bool /*on*/) {}
};

/**
 * @brief Generic IoTTextDisplay adapter for I2C character LCD drivers.
 *
 * Wraps any TDriver whose constructor takes (uint8_t address, uint8_t cols,
 * uint8_t rows) and exposes the standard LCD API (clear, setCursor, print,
 * createChar, write). Per-driver init and backlight differences are handled
 * by IoTLCDDriverTraits<TDriver>.
 *
 * Include the driver-specific adapter header (e.g. IoTLCDDisplay.h or
 * IoTWinstarDisplay.h) rather than this file directly — those headers define
 * the traits specialisation before the using alias, ensuring the correct
 * initialisation sequence is selected.
 *
 * @tparam TDriver  The underlying LCD driver class.
 * @tparam Traits   Traits struct; defaults to IoTLCDDriverTraits<TDriver>.
 */
template<typename TDriver, typename Traits = IoTLCDDriverTraits<TDriver>>
class IoTLCDDisplayT : public IoTTextDisplay
{
public:
    /**
     * @param address  7-bit I2C address of the LCD.
     * @param cols     Number of character columns (e.g. 16).
     * @param rows     Number of character rows (e.g. 2).
     */
    IoTLCDDisplayT(uint8_t address, uint8_t cols, uint8_t rows)
        : _lcd(address, cols, rows), _cols(cols), _rows(rows)
    {
    }

    void begin() override
    {
        Traits::begin(_lcd, _cols, _rows);
    }

    void clear() override
    {
        _lcd.clear();
    }

    uint8_t cols() const override { return _cols; }
    uint8_t rows() const override { return _rows; }

    void setCursor(uint8_t col, uint8_t row) override
    {
        _lcd.setCursor(col, row);
    }

    using IoTTextDisplay::print;

    void print(const char* text) override
    {
        _lcd.print(text);
    }

    void print(const __FlashStringHelper* text) override
    {
        _lcd.print(text);
    }

    void print(int value) override
    {
        _lcd.print(value);
    }

    void print(float value, uint8_t decimals = 1) override
    {
        _lcd.print(value, (int)decimals);
    }

    void setBacklight(bool on) override
    {
        Traits::setBacklight(_lcd, on);
    }

    void createChar(uint8_t index, const uint8_t charmap[8]) override
    {
        // LiquidCrystal_I2C::createChar takes a non-const pointer; cast is safe
        // since the underlying implementation only reads the array.
        _lcd.createChar(index, const_cast<uint8_t*>(charmap));
    }

    void writeChar(uint8_t index) override
    {
        _lcd.write(index);
    }

    void onSystemEvent(const IoTSystemEvent& event) override
    {
        using T = IoTSystemEvent::Type;
        switch (event.type)
        {
            case T::OTA_START:
                _lastOtaPct = 0xFF;
                printLine(0, L_LCD_OTA_START_LINE1);
                printLine(1, L_LCD_OTA_START_LINE2);
                break;

            case T::OTA_PROGRESS:
            {
                uint8_t pct = (event.arg2 > 0)
                    ? (uint8_t)((uint64_t)event.arg1 * 100 / event.arg2) : 0;
                if (_lastOtaPct == 0xFF || pct == 100 || pct >= _lastOtaPct + 20)
                {
                    _lastOtaPct = pct;
                    char buf[17];
                    snprintf(buf, sizeof(buf), L_LCD_OTA_PROGRESS_FMT, pct);
                    printLine(1, buf);
                }
                break;
            }

            case T::OTA_END:
                _lastOtaPct = 0xFF;
                printLine(0, event.flag ? L_LCD_OTA_END_OK : L_LCD_OTA_END_FAIL);
                printLine(1, "");
                break;

            case T::RESTARTING:
                printLine(0, L_LCD_RESTARTING);
                printLine(1, "");
                break;

            default:
                break;
        }
    }

#ifdef _IOT_REAL_TIME
    void printDateTime() override
    {
        if (timeStatus() == timeNotSet)
        {
            printLine(0, "  --:--:--      ");
            printLine(1, "");
            return;
        }

        static const char* const kDayNames[] = { L_LCD_DAY_NAMES };
        time_t t = now();
        char line[17];

        snprintf(line, sizeof(line), "    %02d:%02d:%02d    ",
            hour(t), minute(t), second(t));
        printLine(0, line);

        snprintf(line, sizeof(line), "%s %02d.%02d.%04d",
            kDayNames[weekday(t) - 1], day(t), month(t), year(t));
        printLine(1, line);
    }
#endif

private:
    TDriver  _lcd;
    uint8_t  _cols;
    uint8_t  _rows;
    uint8_t  _lastOtaPct = 0xFF;   // 0xFF = sentinel: no OTA progress shown yet
};
