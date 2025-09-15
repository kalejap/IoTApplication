/*
  IoTApplication.h - Abstract class for IoT application with Wifi connectivity
  and MQTT support for Home Assistent connection.
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

#ifndef IOTAPPLICATION_H
#define IOTAPPLICATION_H

/*
#if defined(ESP8266)
    #include <WiFiClient.h>
    #include <ESP8266WebServer.h>
#elif defined(ESP32)
    #include <WiFiClient.h>
    #include <WebServer.h>
#endif
*/

#include <NTPClient.h>
#include <Timezone.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncDNSServer.h>

#include "IoTDevice.h"
#include "IoTDebug.h"
#include "Timer.h"

#define MAGIC_PREFIX "PK\x7Cpk\x7C#\x0F\x04\x45:"

// Forward declarations
class ESPAsync_WiFiManager;

/**
 * @brief Abstract class to represent an IoT application with WiFi connectivity
 * and MQTT and Home assistant support.
 */
class IoTApplication
{
public:
    IoTApplication(IoTDevice* pIoTDevice);
    virtual ~IoTApplication();

    /**
     * @brief Setup of the application
     */
    virtual void setup();

    /**
     * @brief Main loop of the application
     */
    virtual void loop();

    /**
     * @brief update state machine if update time has elapsed and
     *        update screen.
     * @param bForceUpdate - if true then force update
     */
    void update(bool bForceUpdate = false);

    /**
     * @brief Get current system UTC time
     */
    time_t UTCTime();

    /**
     * @brief Get local time
     */
    time_t localTime();

    /**
     * @brief Convert RSII to quality percantage
    */
    static uint8_t getRSSIasQuality(int8_t RSSI);

private:
    /**
     * @brief Creates WiFi AP to configure WiFi and MQTT
     */
    bool configure();

    /**
     * @brief Publish device to Home Assistant
     */
    void publishDeviceToHA();

    /**
     * @brief Returns WiFi status as string
     */
    String WiFiStatusToString() const;

    /**
     * @brief Callback function to update system time
     */
    static time_t NTPsyncProvider();

    /** 
     * @brief Set system time and register function to periodically update
     * system time from NTC server.
     */
    void setupTime();

    /**
     * @brief get WiFi status character to show on LCD
     */
    char getWifiStatusCharacter() const;

private:
    
    /**
     * @brief Access to device properties
     */
    const DeviceProperties& deviceProperties()
    {
        return _pIoTDevice->deviceProperties();
    }

    /**
     * @brief Pointer to WiFi manager
     */
    ESPAsync_WiFiManager* _pWiFiManager = nullptr;

    /**
     * @brief Pointer to associated IOT device
     */
    IoTDevice* _pIoTDevice;

    /**
     * Time client updating time from NTP
     */
    //static NTPClient _timeClient;

    /**
     * @brief Web server
     */
    AsyncWebServer _webServer;

    /**
     * @brief DNS server
     */
    AsyncDNSServer _dnsServer;

    // Timers
    Timer _automaticUpdateTimer;

    /**
     * @brief Update timer for BME device
     */
    Timer _wifiUpdateTimer;

    /**
     * @brief Time zone
     */
    //Timezone _timeZone;

    // Flag set by WiFiManager's callback function
    bool _saveConfig = false;

    // Flag is set to true when WiFi is in use
    bool _bUsingWiFi = false;

    // Flag to publish device to HA
    bool _bPublishDeviceToHA = false;

    /**
     * @brief Wifi client for ArduinoHA
     */
    //WiFiClient _wifiClient;

    /**
     * @brief MQTT client
     */
    //HAMqtt _mqtt;

    String _verTag;
};

#endif // IOTAPPLICATION_H
