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
 * @brief Class for permanent storing of WiFi settings.
 *        Uses fixed-size char[] instead of String to avoid heap fragmentation.
 */
class WiFiSettings : public Settings
{
public:
    static constexpr size_t SSID_MAX_LEN = 32;
    static constexpr size_t PWD_MAX_LEN  = 63;
    static constexpr size_t IP_MAX_LEN   = 15;

    /**
     * @brief Constructor
     * @param psName - Namespace name
     */
    WiFiSettings(PGM_P psName = "WIFI");

    /**
     * @brief Return SSID (name of WiFi network)
     */
    const char* SSID() const { return m_ssid; }

    /**
     * @brief Set SSID name
     */
    void setSSID(const String& ssid)
    {
        updateValue(ssid.c_str(), m_ssid, sizeof(m_ssid));
    }

    /**
     * @brief Return SSID password
     */
    const char* password() const { return m_password; }

    /**
     * @brief Set SSID password
     */
    void setPassword(const String& password)
    {
        updateValue(password.c_str(), m_password, sizeof(m_password));
    }

    /**
     * @brief Return static IP address
     */
    const char* staticIP() const { return m_staticIP; }

    /**
     * @brief Set static IP address
     */
    void setStaticIP(const String& address)
    {
        String trimmed = address;
        trimmed.trim();
        updateValue(trimmed.c_str(), m_staticIP, sizeof(m_staticIP));
    }

    /**
     * @brief Return static gateway address
     */
    const char* staticGateway() const { return m_staticGateway; }

    /**
     * @brief Set static gateway address
     */
    void setStaticGateway(const String& address)
    {
        String trimmed = address;
        trimmed.trim();
        updateValue(trimmed.c_str(), m_staticGateway, sizeof(m_staticGateway));
    }

    /**
     * @brief Return static subnet mask
     */
    const char* staticSubnet() const { return m_staticSubnet; }

    /**
     * @brief Set static subnet mask
     */
    void setStaticSubnet(const String& addressMask)
    {
        String trimmed = addressMask;
        trimmed.trim();
        updateValue(trimmed.c_str(), m_staticSubnet, sizeof(m_staticSubnet));
    }

protected:
    void readFields(Preferences& pref) override;
    bool saveFields(Preferences& pref) const override;

private:
    char m_ssid[SSID_MAX_LEN + 1]        = {};
    char m_password[PWD_MAX_LEN + 1]     = {};
    char m_staticIP[IP_MAX_LEN + 1]      = {};
    char m_staticGateway[IP_MAX_LEN + 1] = {};
    char m_staticSubnet[IP_MAX_LEN + 1]  = {};
};

#endif // WIFISETTINGS_H
