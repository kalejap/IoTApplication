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

#ifndef WIFISETTINGS_H
#define WIFISETTINGS_H

#include "Settings.h"

/**
 * @brief Class for permanent storing of WIFI and MQTT settings
 */
class WiFiSettings : public Settings
{
public:
    /**
     * @brief Constructor
     * @param psName - Namespace name
     */
    WiFiSettings(PGM_P psName = "WIFI");

    /**
     * @brief Return SSID (name of WIFI network)
     * @return SSID name
     */
    const String& SSID() const 
    {
        return m_ssid;
    }

    /**
     * @brief Set SSID name
     * @param ssid - SSID name
     */
    void setSSID(const String& ssid)
    {
        updateValue(ssid, m_ssid);
    }

    /**
     * @brief Return SSID password
     * @return SSID password
     */
    const String& password() const 
    {
        return m_password;
    }

    /**
     * @brief Set SSID password
     * @param ssid - SSID password
     */
    void setPassword(const String& password)
    {
        updateValue(password, m_password);
    }

    /**
     * @brief Return static IP address
     * @return static IP address
     */
    const String& staticIP() const 
    {
        return m_staticIP;
    }

    /**
     * @brief Set static IP address
     * @param address static IP address
     */
    void setStaticIP(const String& address)
    {
        updateValue(address, m_staticIP, true);
    }

    /**
     * @brief Return static gateway address
     * @return static gateway address
     */
    const String& staticGateway() const 
    {
        return m_staticGateway;
    }

    /**
     * @brief Set static gateway address
     * @param address static gateway address
     */
    void setStaticGateway(const String& address)
    {
        updateValue(address, m_staticGateway, true);
    }

    /**
     * @brief Return static subnet mask
     * @return static subnet mask
     */
    const String& staticSubnet() const 
    {
        return m_staticSubnet;
    }

    /**
     * @brief Set MQTT user name
     * @param addressMask - MQTT user name
     */
    void setStaticSubnet(const String& addressMask)
    {
        updateValue(addressMask, m_staticSubnet, true);
    }

 protected:
    void readFields(Preferences& pref) override;
    bool saveFields(Preferences& pref) const override;

private:
    String m_ssid;
    String m_password;
    String m_staticIP;
    String m_staticGateway;
    String m_staticSubnet;
 };

#endif // WIFISETTINGS_H
