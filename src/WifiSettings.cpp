/*
  WifiSettings.h - Class to manage wifi settings persistently stored
  as prefereneces in Non-volatile space (NVS) of ESP32/ESP8266

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

#include "WifiSettings.h"

WiFiSettings::WiFiSettings(PGM_P psName) :
    Settings(psName)
{}

void WiFiSettings::readFields(Preferences& pref)
{
    m_ssid = pref.getString(PREF_WIFI_SETTING_SSID, "");
    m_password = pref.getString(PREF_WIFI_SETTING_PASSWORD, "");
    m_mqttServer = pref.getString(PREF_WIFI_SETTING_MQTT_SERVER, "");
    m_mqttPort = pref.getUShort(PREF_WIFI_SETTING_MQTT_PORT, 1883);
    m_mqttUser = pref.getString(PREF_WIFI_SETTING_MQTT_USER, "");
    m_mqttPassword = pref.getString(PREF_WIFI_SETTING_MQTT_PASSWORD, "");
}

bool WiFiSettings::saveFields(Preferences& pref) const
{
    pref.putString(PREF_WIFI_SETTING_SSID, m_ssid);
    pref.putString(PREF_WIFI_SETTING_PASSWORD, m_password);
    pref.putString(PREF_WIFI_SETTING_MQTT_SERVER, m_mqttServer);
    pref.putUShort(PREF_WIFI_SETTING_MQTT_PORT, m_mqttPort);
    pref.putString(PREF_WIFI_SETTING_MQTT_USER, m_mqttUser);
    pref.putString(PREF_WIFI_SETTING_MQTT_PASSWORD, m_mqttPassword);
    return true;
}
