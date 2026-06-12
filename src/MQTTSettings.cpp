/*
  MQTTSettings.h - Class to manage wifi settings persistently stored
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

#include "MQTTSettings.h"

#define PREF_MQTT_SETTING_SERVER "MQTTSERVER"
#define PREF_MQTT_SETTING_PORT "MQTTPORT"
#define PREF_MQTT_SETTING_USER "MQTTUSER"
#define PREF_MQTT_SETTING_PASSWORD "MQTTPWD"


MQTTSettings::MQTTSettings(PGM_P psName) :
    Settings(psName)
{}

void MQTTSettings::readFields(Preferences& pref)
{
    m_mqttServer = pref.getString(PREF_MQTT_SETTING_SERVER, "");
    m_mqttPort = pref.getUShort(PREF_MQTT_SETTING_PORT, 1883);
    m_mqttUser = pref.getString(PREF_MQTT_SETTING_USER, "");
    m_mqttPassword = pref.getString(PREF_MQTT_SETTING_PASSWORD, "");
}

bool MQTTSettings::saveFields(Preferences& pref) const
{
    pref.putString(PREF_MQTT_SETTING_SERVER, m_mqttServer);
    pref.putUShort(PREF_MQTT_SETTING_PORT, m_mqttPort);
    pref.putString(PREF_MQTT_SETTING_USER, m_mqttUser);
    pref.putString(PREF_MQTT_SETTING_PASSWORD, m_mqttPassword);

    return true;
}
