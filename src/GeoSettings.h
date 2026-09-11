/*
  GeoSettings.h - Class to manage geographic location settings persistently
  stored as preferences in Non-volatile space (NVS) of ESP32/ESP8266.

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

#ifndef GEOSETTINGS_H
#define GEOSETTINGS_H

#include "Settings.h"

/**
 * @brief NVS-backed geographic location settings (latitude, longitude, altitude).
 *        Stored in namespace "GEO" with float keys "LAT", "LNG", "ALT".
 *        Enabled only when WM_GEO_LOCATION is defined.
 */
class GeoSettings : public Settings
{
public:
    /**
     * @brief Constructor
     * @param psName - Namespace name (default "GEO")
     */
    GeoSettings(PGM_P psName = "GEO");

    /**
     * @brief Return latitude in decimal degrees
     */
    float latitude() const { return m_latitude; }

    /**
     * @brief Set latitude in decimal degrees
     */
    void setLatitude(float value) { updateValue(value, m_latitude); }

    /**
     * @brief Return longitude in decimal degrees
     */
    float longitude() const { return m_longitude; }

    /**
     * @brief Set longitude in decimal degrees
     */
    void setLongitude(float value) { updateValue(value, m_longitude); }

    /**
     * @brief Return altitude in metres above sea level
     */
    float altitude() const { return m_altitude; }

    /**
     * @brief Set altitude in metres above sea level
     */
    void setAltitude(float value) { updateValue(value, m_altitude); }

protected:
    void readFields(Preferences& pref) override;
    bool saveFields(Preferences& pref) const override;

private:
    float m_latitude  = 0.0f;
    float m_longitude = 0.0f;
    float m_altitude  = 0.0f;
};

#endif // GEOSETTINGS_H
