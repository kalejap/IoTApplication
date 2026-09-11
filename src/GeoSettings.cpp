/*
  GeoSettings.cpp - Class to manage geographic location settings persistently
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

#include "GeoSettings.h"

#define PREF_GEO_LATITUDE  "LAT"
#define PREF_GEO_LONGITUDE "LNG"
#define PREF_GEO_ALTITUDE  "ALT"

GeoSettings::GeoSettings(PGM_P psName) :
    Settings(psName)
{}

void GeoSettings::readFields(Preferences& pref)
{
    m_latitude  = pref.getFloat(PREF_GEO_LATITUDE,  0.0f);
    m_longitude = pref.getFloat(PREF_GEO_LONGITUDE, 0.0f);
    m_altitude  = pref.getFloat(PREF_GEO_ALTITUDE,  0.0f);
}

bool GeoSettings::saveFields(Preferences& pref) const
{
    pref.putFloat(PREF_GEO_LATITUDE,  m_latitude);
    pref.putFloat(PREF_GEO_LONGITUDE, m_longitude);
    pref.putFloat(PREF_GEO_ALTITUDE,  m_altitude);
    return true;
}
