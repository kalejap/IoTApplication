/*
  MQTTSettings.h - Class to manage MQTT settings persistently stored
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

#ifndef MQTTSETTINGS_H
#define MQTTSETTINGS_H

#include "Settings.h"

/**
 * @brief Class for permanent storing of MQTT settings.
 *        Uses fixed-size char[] instead of String to avoid heap fragmentation.
 */
class MQTTSettings : public Settings
{
public:
    static constexpr size_t SERVER_MAX_LEN = 64;
    static constexpr size_t USER_MAX_LEN   = 64;
    static constexpr size_t PWD_MAX_LEN    = 64;

    /**
     * @brief Constructor
     * @param psName - Namespace name
     */
    MQTTSettings(PGM_P psName = "MQTT");

    /**
     * @brief Return MQTT server name
     */
    const char* MQTTServer() const { return m_mqttServer; }

    /**
     * @brief Set MQTT server name
     */
    void setMQTTServer(const String& name)
    {
        String trimmed = name;
        trimmed.trim();
        updateValue(trimmed.c_str(), m_mqttServer, sizeof(m_mqttServer));
    }

    /**
     * @brief Return MQTT port
     */
    uint16_t MQTTPort() const { return m_mqttPort; }

    /**
     * @brief Set MQTT port
     */
    void setMQTTPort(uint16_t port)
    {
        updateValue(port, m_mqttPort);
    }

    /**
     * @brief Return MQTT user name
     */
    const char* MQTTUser() const { return m_mqttUser; }

    /**
     * @brief Set MQTT user name
     */
    void setMQTTUser(const String& name)
    {
        String trimmed = name;
        trimmed.trim();
        updateValue(trimmed.c_str(), m_mqttUser, sizeof(m_mqttUser));
    }

    /**
     * @brief Return MQTT password
     */
    const char* MQTTPassword() const { return m_mqttPassword; }

    /**
     * @brief Set MQTT password
     */
    void setMQTTPassword(const String& password)
    {
        String trimmed = password;
        trimmed.trim();
        updateValue(trimmed.c_str(), m_mqttPassword, sizeof(m_mqttPassword));
    }

protected:
    void readFields(Preferences& pref) override;
    bool saveFields(Preferences& pref) const override;

private:
    char     m_mqttServer[SERVER_MAX_LEN + 1] = {};
    uint16_t m_mqttPort                        = 1883;
    char     m_mqttUser[USER_MAX_LEN + 1]     = {};
    char     m_mqttPassword[PWD_MAX_LEN + 1]  = {};
};

#endif // MQTTSETTINGS_H
