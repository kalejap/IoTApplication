
/*
  AppSettings.h - Application-level settings persistently stored in NVS.
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

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "Settings.h"

/**
 * @brief Application-level settings stored in NVS namespace "APP".
 *
 * Currently holds the global temperature unit preference (°C / °F)
 * which applies to all temperature sensors in the device.
 *
 * Static utility methods (convertTemperature, temperatureUnit) are
 * provided so sensor classes can perform conversions without needing
 * a reference to an AppSettings instance.
 */
class AppSettings : public Settings
{
public:
    AppSettings();

    /**
     * @brief Whether the global temperature unit is Celsius.
     * @return true = °C (default), false = °F.
     */
    bool temperatureInCelsius() const { return m_temperatureCelsius; }

    /**
     * @brief Set the global temperature unit preference.
     * @param inCelsius  true = °C, false = °F.
     */
    void setTemperatureInCelsius(bool inCelsius);

    /**
     * @brief Convert a Celsius value to the requested unit.
     *
     * @param celsius    Raw temperature from hardware (always in °C).
     * @param inCelsius  true = return value unchanged, false = convert to °F.
     * @return Temperature in the requested unit.
     */
    static float convertTemperature(float celsius, bool inCelsius);

    /**
     * @brief Return the unit-of-measurement label for the given unit preference.
     *
     * The returned pointer is a string literal with static storage duration —
     * safe to pass to setUnitOfMeasurement() and store long-term.
     *
     * @param inCelsius  true → "°C", false → "°F".
     */
    static const char* temperatureUnit(bool inCelsius);

protected:
    void readFields(Preferences& pref) override;
    bool saveFields(Preferences& pref) const override;

private:
    bool m_temperatureCelsius = true;
};

#endif // APPSETTINGS_H
