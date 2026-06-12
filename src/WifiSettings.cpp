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

#define PREF_WIFI_SETTING_SSID "SSID"
#define PREF_WIFI_SETTING_PASSWORD "PWD"
#define PREF_WIFI_SETTING_STATIC_IP "STATICIP"
#define PREF_WIFI_SETTING_STATIC_GATEWAY "STATICGW"
#define PREF_WIFI_SETTING_STATIC_SUBNET "STATICSUBNET"


WiFiSettings::WiFiSettings(PGM_P psName) :
    Settings(psName)
{}

void WiFiSettings::readFields(Preferences& pref)
{
    m_ssid = pref.getString(PREF_WIFI_SETTING_SSID, "");
    m_password = pref.getString(PREF_WIFI_SETTING_PASSWORD, "");
    String m_staticIP;
    String m_staticGateway;
    String m_staticSubnet;

    m_staticIP = pref.getString(PREF_WIFI_SETTING_STATIC_IP, "");
    m_staticGateway = pref.getString(PREF_WIFI_SETTING_STATIC_GATEWAY, "");
    m_staticSubnet = pref.getString(PREF_WIFI_SETTING_STATIC_SUBNET, "");
}

bool WiFiSettings::saveFields(Preferences& pref) const
{
    pref.putString(PREF_WIFI_SETTING_SSID, m_ssid);
    pref.putString(PREF_WIFI_SETTING_PASSWORD, m_password);

    pref.putString(PREF_WIFI_SETTING_STATIC_IP, m_staticIP);
    pref.putString(PREF_WIFI_SETTING_STATIC_GATEWAY, m_staticGateway);
    pref.putString(PREF_WIFI_SETTING_STATIC_SUBNET, m_staticSubnet);
 
    return true;
}
