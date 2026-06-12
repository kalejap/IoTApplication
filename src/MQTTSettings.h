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

#ifndef MQTTSETTINGS_H
#define MQTTSETTINGS_H

#include "Settings.h"

/**
 * @brief Class for permanent storing of MQTT settings
 */
class MQTTSettings : public Settings
{
public:
    /**
     * @brief Constructor
     * @param psName - Namespace name
     */
    MQTTSettings(PGM_P psName = "MQTT");

    /**
     * @brief Return MQTT server name
     * @return MQTT server name
     */
    const String& MQTTServer() const 
    {
        return m_mqttServer;
    }

    /**
     * @brief Set MQTT server name
     * @param name - MQTT server name
     */
    void setMQTTServer(const String& name)
    {
        updateValue(name, m_mqttServer, true);
    }

    /**
     * @brief Return MQTT port
     * @return MQTT port
     */
    uint16_t MQTTPort() const 
    {
        return m_mqttPort;
    }

    /**
     * @brief Set MQTT port
     * @param port - MQTT port
     */
    void setMQTTPort(uint16_t port)
    {
        updateValue(port, m_mqttPort);
    }

    /**
     * @brief Return MQTT user name
     * @return MQTT user name
     */
    const String& MQTTUser() const 
    {
        return m_mqttUser;
    }

    /**
     * @brief Set MQTT user name
     * @param name - MQTT user name
     */
    void setMQTTUser(const String& name)
    {
        updateValue(name, m_mqttUser, true);
    }

    /**
     * @brief Return MQTT password
     * @return MQTT password
     */
    const String& MQTTPassword() const 
    {
        return m_mqttPassword;
    }

    /**
     * @brief Set MQTT password
     * @param password - MQTT password
     */
    void setMQTTPassword(const String& password)
    {
        updateValue(password, m_mqttPassword, true);
    }

protected:
    void readFields(Preferences& pref) override;
    bool saveFields(Preferences& pref) const override;

private:
    String m_mqttServer;
    uint16_t m_mqttPort = 1883;
    String m_mqttUser;
    String m_mqttPassword;
};

#endif // MQTTSETTINGS_H
