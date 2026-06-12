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
#include "AppSettings.h"
#include "ESPAsync_WiFiManagerUtils.h"

//#define MAGIC_PREFIX "\xcc\x80\xc1\xca\x6e\xf3\x49\x7f\xa7\x26"

// Forward declarations
class IoTWiFiManager;

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

    /**
     * @brief Method to handle static file request, return true if handled
     */
    virtual bool handleStaticFileRequest(AsyncWebServerRequest *request);

    /**
     * @brief Method to handle system query, return true if handled
     */
    virtual bool handleCustomSystemQuery(AsyncWebServerRequest *request);

    /**
     * @brief Access to application-level settings (temperature unit, etc.).
     */
    const AppSettings& appSettings() const { return _appSettings; }


private:
    /**
     * @brief Creates WiFi AP to configure WiFi and MQTT
     */
    bool configure();

    /**
     * @brief Register common static asset routes (e.g. /hw-status.js).
     *        Called from both the STA and AP setup paths.
     */
    void _registerCommonRoutes();

    /**
     * @brief Register GET "/" handler with the given filter (AP or STA).
     */
    void _registerRootHandler(ArRequestFilterFunction filter);

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

    /**
     * @brief Register OTA, WiFi and MQTT system event callbacks.
     *        Called once from both setup() and configure() before the
     *        WiFiManager is started.
     */
    void _registerSystemEventCallbacks();

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
    IoTWiFiManager* _pWiFiManager = nullptr;

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
     * @brief Display refresh timer — drives IoTDevice::_tickDisplay() every 2 s.
     */
    Timer _displayUpdateTimer;

    /**
     * @brief Time zone
     */
    //Timezone _timeZone;

    // Application-level settings (temperature unit, etc.)
    AppSettings _appSettings;

    // Flag set by WiFiManager's callback function
    bool _saveConfig = false;

    // Flag is set to true when WiFi is in use
    bool _bUsingWiFi = false;

    // Flag to publish device to HA
    bool _bPublishDeviceToHA = false;

#ifdef ESP8266
    // Held alive for the lifetime of the application (handlers deregister on destruction).
    WiFiEventHandler _wifiConnectHandler;
    WiFiEventHandler _wifiDisconnectHandler;
#endif

#ifdef WM_SUPPORT_HOME_ASSISTANT
    bool _mqttWasConnected = false;
#endif

#ifdef WM_SUPPORT_HOME_ASSISTANT
    /**
     * @brief Wifi client for ArduinoHA
     */
    WiFiClient _wifiClient;

    /**
     * @brief
     */
    HADevice _haDevice;

    /**
     * @brief MQTT client
     */
    HAMqtt _mqtt;
#endif
};

#endif // IOTAPPLICATION_H
