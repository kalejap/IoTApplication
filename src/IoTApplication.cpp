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


#include <WiFiUdp.h>
//#include <ElegantOTA.h>
#include <TimeLib.h>
#include <IoTWiFiManager-Impl.h>

#include "IoTApplication.h"
#include "WifiSettings.h"
#include "MQTTSettings.h"
#include "Version.h"

/////////////////////////////////////////////////////////////////////
//
// Global variables
//
/////////////////////////////////////////////////////////////////////

namespace
{
    /*
    bool initTime(NTPClient& timeClient)
    {
        timeClient.end();
        timeClient.begin();

        if (!timeClient.forceUpdate())
            return false;

        int8_t attempts = 3;
        while (attempts)
        {
            unsigned long t1 = timeClient.getEpochTime();
            delay(100);
            if (timeClient.forceUpdate())
            {
                unsigned long t2 = timeClient.getEpochTime();
                if (t2 - t1 < 5) // if time difference is less than 5 sec
                    return true;
            }
            attempts--;
        }

        return false;
    }
   
    WiFiUDP ntpUDP;
    */

    unsigned long ota_progress_millis = 0;
}

//NTPClient IoTApplication::_timeClient(ntpUDP, "europe.pool.ntp.org", 0, 300000);


IoTApplication::IoTApplication(IoTDevice* pIoTDevice) :
    _pIoTDevice(pIoTDevice),
    _webServer(80),
    //_wm(&_webServer, &_dnsServer),
    _automaticUpdateTimer(15*1000),
    _wifiUpdateTimer(60*1000),
    _displayUpdateTimer(2000)
    //_timeZone(pIoTDevice->deviceProperties().dstStart, pIoTDevice->deviceProperties().stdStart),
#ifdef WM_SUPPORT_HOME_ASSISTANT
    ,_mqtt(_wifiClient, pIoTDevice->device())
#endif
{
    _pWiFiManager = new IoTWiFiManager(&_webServer, &_dnsServer);
    _pWiFiManager->setApplication(this);
    _pWiFiManager->setHardwareId(pIoTDevice->deviceProperties().hardwareId);
}

IoTApplication::~IoTApplication()
{
    delete _pWiFiManager;
}

void IoTApplication::_registerCommonRoutes()
{
    _webServer.on("/hw-status.js", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send_P(200, "application/javascript", WM_PK_HW_STATUS_JS);
    });
}

void IoTApplication::_registerRootHandler(ArRequestFilterFunction filter)
{
    _webServer.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        ESPAsync_WiFiManagerUtils::responseText(req, &gIndexHtml);
    }).setFilter(filter);
}

void IoTApplication::_registerSystemEventCallbacks()
{
    _pWiFiManager->onOTAStart([this]() {
        _pIoTDevice->onSystemEvent({IoTSystemEvent::Type::OTA_START});
    });
    _pWiFiManager->onOTAProgress([this](size_t current, size_t total) {
        IoTSystemEvent e;
        e.type = IoTSystemEvent::Type::OTA_PROGRESS;
        e.arg1 = (uint32_t)current;
        e.arg2 = (uint32_t)total;
        _pIoTDevice->onSystemEvent(e);
    });
    _pWiFiManager->onOTAEnd([this](bool success) {
        IoTSystemEvent e;
        e.type = IoTSystemEvent::Type::OTA_END;
        e.flag = success;
        _pIoTDevice->onSystemEvent(e);
    });

#ifdef ESP8266
    _wifiConnectHandler = WiFi.onStationModeGotIP(
        [this](const WiFiEventStationModeGotIP& ev) {
            IoTSystemEvent e;
            e.type = IoTSystemEvent::Type::WIFI_CONNECTED;
            e.arg1 = (uint32_t)ev.ip;
            _pIoTDevice->onSystemEvent(e);
        });
    _wifiDisconnectHandler = WiFi.onStationModeDisconnected(
        [this](const WiFiEventStationModeDisconnected&) {
            _pIoTDevice->onSystemEvent({IoTSystemEvent::Type::WIFI_DISCONNECTED});
        });
#endif
}

void IoTApplication::setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("IoTApplication::setup()");

    _appSettings.read();
    IOTLOGINFO1(F("Temperature unit: "), _appSettings.temperatureInCelsius() ? F("Celsius") : F("Fahrenheit"));

#ifdef WM_SUPPORT_HOME_ASSISTANT
    MQTTSettings mqttSettings;
    mqttSettings.read();
#endif

    _pIoTDevice->preSetup();
    _pIoTDevice->postSetup();

    /*
    pinMode(PIN_MODE_BUTTON, INPUT); // config GPIO34 as input pin (it does not have any built-in pull-up/pulldown resistor)
    pinMode(PIN_ROTARY_ENCODER_A, INPUT);
    pinMode(PIN_ROTARY_ENCODER_B, INPUT);

    delay(1000);

    // Completely erase NVS
    //nvs_flash_erase();      // erase the NVS partition and...
    //nvs_flash_init();       // initialize the NVS partition.

    m_lcd.init();

    m_lcd.createChar(DA_CODE_TO_CHAR(DA_SMALL_CH), char_small_ch);
    m_lcd.createChar(DA_CODE_TO_CHAR(DA_LEVEL_UP), levelUp);
    m_lcd.createChar(DA_CODE_TO_CHAR(DA_RADIO_BUTTON_OFF), radioButtonOff);
    m_lcd.createChar(DA_CODE_TO_CHAR(DA_RADIO_BUTTON_ON), radioButtonOn);
    m_lcd.createChar(DA_CODE_TO_CHAR(DA_CHECK_ON), checkOn);
    m_lcd.createChar(DA_CODE_TO_CHAR(DA_WIFI_SIGNAL_EXCELLENT), wifiSignalExcellent); 
    m_lcd.createChar(DA_CODE_TO_CHAR(DA_WIFI_SIGNAL_GOOD), wifiSignalExcellent);
    m_lcd.createChar(DA_CODE_TO_CHAR(DA_THERMOMETER), thermometer);

    m_lcd.backlight();

    m_lcd.clear();
    m_lcd.print("Solar Controller DYI");
    m_lcd.setCursor(0, 1);
    m_lcd.print(String("Verzia: ") + s_version);
    m_lcd.setCursor(0, 2);
    m_lcd.print("Inicializacia...");

    Serial.println("Inicializacia...");

    */

    // Setup SPI
    //SPI.begin();

    delay(1000);

    WiFiSettings wifiSettings("WIFI");
    // Read WiFi settings
    wifiSettings.read();

    // Check request for configuration via access point
    if (wifiSettings.SSID().isEmpty() || _pIoTDevice->isConfigTriggeredOnStartUp())
    {
        configure();
    }

    // Timer timers
   
    String ssid(wifiSettings.SSID());
    String pwd(wifiSettings.password());
    if (!ssid.isEmpty())
    {
        IOTLOGDEBUG1(F("SSIS: "), ssid);
        IOTLOGDEBUG1(F("Password: "), pwd);
        //WiFi.hostname("_IoTApplicationTest1");
        //WiFi.setPhyMode(WIFI_PHY_MODE_11N);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pwd.c_str());
        WiFi.setSleep(false);

        //
        IOTLOGDEBUG(F("Connecting to WiFi: "));
        uint8_t attempts = 0;
        while ( WiFi.status() != WL_CONNECTED &&
                attempts < 5)
        {
            Serial.print (".");
            delay (1000);
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            IOTLOGDEBUG(F(". Connected"));
            IOTLOGDEBUG1(F("IP:"), WiFi.localIP());
        }
        else
        {
            IOTLOGDEBUG(F(". Not connected"));
        }

        delay(1000);
    
        _bUsingWiFi = true;

        /*
        byte str[16] = { 0xA, 0x1, 0x5, 0xB, 0xF, 0x4, 0x2, 0xF, 0xE, 0xB, 0x2, 0x6, 0x8, 0x7, 0x9, 0x3 };
        m_HAdevice.setUniqueId(str, 16);

        // Set other device properties
        m_HAdevice.setName("Solar Controller Test 1");
        m_HAdevice.setModel("SCTest1");
        m_HAdevice.setSoftwareVersion("1.0.0");
        m_HAdevice.setAvailability(true);
        m_HAdevice.setManufacturer("DIY");
        */

        // MQTT broker connection (use your data here)
        //m_mqtt.begin(HA_MQTT_BROKER, HA_MQTT_PORT, HA_MQTT_USER, HA_MQTT_PASSWORD);

    #ifdef WM_SUPPORT_HOME_ASSISTANT
        IPAddress ipAddress;
        if (ipAddress.fromString(mqttSettings.MQTTServer().c_str()))
        {
            // MQTT broker connection using IP address
            _mqtt.begin(ipAddress, mqttSettings.MQTTPort(),
                mqttSettings.MQTTUser().c_str(), mqttSettings.MQTTPassword().c_str());
            IOTLOGINFO("MQTT connecting using IP address");
        }
        else
        {
            // MQTT broker connection using server name
            _mqtt.begin(mqttSettings.MQTTServer().c_str(), mqttSettings.MQTTPort(),
                mqttSettings.MQTTUser().c_str(), mqttSettings.MQTTPassword().c_str());
            IOTLOGINFO("MQTT connecting using server name");
        }
    #endif

        // Configure sensors
        /*
        m_tankTempSensor.setIcon("mdi:thermometer");
        m_tankTempSensor.setName("Teplota nádrže");
        m_tankTempSensor.setUnitOfMeasurement("°C");
        m_tankTempSensor.setDeviceClass("temperature");
        m_tankTempSensor.setForceUpdate(true);

        m_tankErrorSensor.setName("Sensor teploty nádrže");
        m_tankErrorSensor.setForceUpdate(true);
        */

        /*
        _webServer.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
            String str(F("Hi! This is ElegantOTA AsyncDemo.\r\n"));
            str += F("Version: ");
            str += FPSTR(this->_pIoTDevice->deviceProperties().versionString);
            request->send(200, "text/plain", str);
        });
        */

        _pIoTDevice->onAddConfigRoutes(_webServer);
        _registerCommonRoutes();
        _registerRootHandler(ON_STA_FILTER);
        _registerSystemEventCallbacks();

        _pWiFiManager->handleSTA();

        _webServer.on("/api/appsave", HTTP_POST, [this](AsyncWebServerRequest *request) {
            if (request->hasArg("temp_unit"))
            {
                const String unit = request->arg("temp_unit");
                _appSettings.setTemperatureInCelsius(unit != "F");
                if (_appSettings.isDirty())
                {
                    _appSettings.save();
                    IOTLOGINFO1(F("Temperature unit saved: "), unit);
                }
            }
            ESPAsync_WiFiManagerUtils::responseApplJson(request, String(F("{\"result\":\"ok\"}")));
        });

        /*
        ElegantOTA.begin(&_webServer);    // Start ElegantOTA
        // ElegantOTA callbacks
        ElegantOTA.onStart([]() {
            // Log when OTA has started
            Serial.println("OTA update started!");
            // <Add your own code here>
        });
        ElegantOTA.onProgress([](size_t current, size_t final) {
            // Log every 1 second
            if (millis() - ota_progress_millis > 1000) {
                ota_progress_millis = millis();
                Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
            }
        });
        ElegantOTA.onEnd([](bool success) {
            // Log when OTA has finished
            if (success) {
                Serial.println("OTA update finished successfully!");
            } else {
                Serial.println("There was an error during OTA update!");
            }
            // <Add your own code here>
        });
        */
       
        _webServer.begin();
    }

    // Setup time 
    setupTime();

#ifdef WM_SUPPORT_HOME_ASSISTANT
    IOTLOGINFO2(F("MQTT server: |"), mqttSettings.MQTTServer(), F("|"));
    IOTLOGINFO2(F("MQTT port: |"), mqttSettings.MQTTPort(), F("|"));
    IOTLOGINFO2(F("MQTT user: |"), mqttSettings.MQTTUser(), F("|"));
    IOTLOGINFO2(F("MQTT password: |"), mqttSettings.MQTTPassword(), F("|"));

    IOTLOGINFO1(F("_ESPASYNC_WIFIMGR_LOGLEVEL_: "), _ESPASYNC_WIFIMGR_LOGLEVEL_);
#endif // WM_SUPPORT_HOME_ASSISTANT

    delay(500);

    // Prime all components so the display shows real values on the first tick
    // instead of "------" for up to 15 seconds (the automatic update timer period).
#ifdef WM_SUPPORT_HOME_ASSISTANT
    _pIoTDevice->updateAllComponents(true);
#endif
}

void IoTApplication::loop()
{
    _pWiFiManager->loop();
    _pIoTDevice->preLoop();

#ifdef WM_SUPPORT_HOME_ASSISTANT
    if (_bUsingWiFi)
    {
        _mqtt.loop();
    }
#endif

    update();

    _pIoTDevice->postLoop();
}

void IoTApplication::update(bool bForceUpdate)
{
    if (_displayUpdateTimer.elapsed())
        _pIoTDevice->_tickDisplay();

#ifdef WM_SUPPORT_HOME_ASSISTANT
    if (_bUsingWiFi)
    {
        bool mqttNowConnected = _mqtt.isConnected();
        if (mqttNowConnected != _mqttWasConnected)
        {
            _mqttWasConnected = mqttNowConnected;
            IoTSystemEvent e;
            e.type = mqttNowConnected
                ? IoTSystemEvent::Type::MQTT_CONNECTED
                : IoTSystemEvent::Type::MQTT_DISCONNECTED;
            _pIoTDevice->onSystemEvent(e);
        }
    }
#endif

    if (!bForceUpdate && !_automaticUpdateTimer.elapsed())
        return;

    if (bForceUpdate)
        _automaticUpdateTimer.restart();

#ifdef WM_SUPPORT_HOME_ASSISTANT
    _pIoTDevice->updateAllComponents(bForceUpdate);
    if (_bUsingWiFi)
        _pIoTDevice->publishAllComponents(bForceUpdate);
#endif
}


bool IoTApplication::configure()
{
    WiFiSettings wifiSettings("WIFI");


    _saveConfig = false;
    _pWiFiManager->setSaveConfigCallback([this](){
        this->_saveConfig = true;
    });

    //reset settings - for testing
    //wm.resetSettings();

    /*
    m_lcd.clear();
    //           01234567890123456789
    m_lcd.print("Rezim: WiFi AP");
    m_lcd.setCursor(0, 1);
    m_lcd.print("SSID: ");
    m_lcd.print(kAccessPointName);
    m_lcd.setCursor(0, 2);
    m_lcd.print("Heslo: <ziadne>");
    m_lcd.setCursor(0, 3);
    m_lcd.print("IP: 192.168.4.1");
    */
 
    // set configportal timeout in seconds
    _pWiFiManager->setConfigPortalTimeout(180); // 3 min

#ifdef WM_SUPPORT_HOME_ASSISTANT
    MQTTSettings mqttSettings;
    mqttSettings.read();
    ESPAsync_WMParameter customMQTTserver("mqtt_server", "MQTT server", mqttSettings.MQTTServer().c_str(), 40);
    ESPAsync_WMParameter customMQTTport("mqtt_port", "MQTT port", String(mqttSettings.MQTTPort()).c_str(), 40);
    ESPAsync_WMParameter customMQTTuser("mqtt_user", "MQTT user", mqttSettings.MQTTUser().c_str(), 40);
    ESPAsync_WMParameter customMQTTpwd("mqtt_pwd", "MQTT password", mqttSettings.MQTTPassword().c_str(), 40);
    _pWiFiManager->addParameter(&customMQTTserver);
    _pWiFiManager->addParameter(&customMQTTport);
    _pWiFiManager->addParameter(&customMQTTuser);
    _pWiFiManager->addParameter(&customMQTTpwd);
#endif

    // Allow the device to register extra HTTP routes (e.g. live JSON endpoints, static assets).
    _pIoTDevice->onAddConfigRoutes(_webServer);
    _registerCommonRoutes();
    _registerRootHandler(ON_AP_FILTER);

    // Inject optional <head> element (e.g. <script src> + <style>) from the device.
    const char* headElem = _pIoTDevice->onCustomHeadElement();
    if (headElem) _pWiFiManager->setCustomHeadElement(headElem);

    // Let the device append custom HTML (table, buttons, inline <script>, …).
    String deviceConfigHtml;
    _pIoTDevice->onBuildConfigHtml(deviceConfigHtml);
    ESPAsync_WMParameter deviceParam(deviceConfigHtml.c_str());
    if (deviceConfigHtml.length() > 0) _pWiFiManager->addParameter(&deviceParam);

#ifdef ESP8266
    String ssid = "ESP_" + String(ESP.getChipId());
#else   //ESP32
    String ssid = "ESP_" + String(ESP_getChipId());
#endif

    ssid.toUpperCase();

    _pIoTDevice->onConfig(ssid);
    _registerSystemEventCallbacks();

    if (!_pWiFiManager->startConfigPortal(ssid.c_str()))
    {
        IOTLOGDEBUG(F("failed to connect and hit timeout"));
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);

        return false;
    }

    IOTLOGDEBUG1(F("_saveConfig: "), _saveConfig);
     
    if (_saveConfig)
    {
        IOTLOGDEBUG(F("Saving wifi configuration: "));
        IOTLOGDEBUG1(F("SSID: "), _pWiFiManager->getSSID());
        IOTLOGDEBUG1(F("PWD: "), _pWiFiManager->getPW());
        // Copy acquired settings to m_wifiSettings and save it
        wifiSettings.setSSID(_pWiFiManager->getSSID());
        wifiSettings.setPassword(_pWiFiManager->getPW());

    #ifdef WM_SUPPORT_HOME_ASSISTANT
        mqttSettings.setMQTTServer(customMQTTserver.getValue());
        mqttSettings.setMQTTPort(String(customMQTTport.getValue()).toInt());
        mqttSettings.setMQTTUser(customMQTTuser.getValue());
        mqttSettings.setMQTTPassword(customMQTTpwd.getValue());
        if (mqttSettings.isDirty()) mqttSettings.save();
    #endif

        if (wifiSettings.isDirty())
        {
            wifiSettings.save();
            IOTLOGDEBUG(F("WiFi configuration saved."));
        }

        // Allow the device to read custom parameter values and persist them.
        _pIoTDevice->onSaveConfigParameters();

        return true;
    }

    return false;
}

void IoTApplication::setupTime()
{
    /*
    if (initTime(_timeClient))
    {
        setTime(_timeClient.getEpochTime());
        setSyncProvider(NTPsyncProvider);
    }
    else
    {
        tmElements_t tm;
        tm.Second = 0;
        tm.Minute = 0;
        tm.Hour = 21;
        tm.Day = 3;
        tm.Month = 5;
        tm.Year = 50;
        setTime(makeTime(tm));
    }
    */
}

time_t IoTApplication::NTPsyncProvider()
{
    return 0;
    //return (_timeClient.update()) ? _timeClient.getEpochTime() : 0;
}

time_t IoTApplication::UTCTime()
{
    return now();
}

time_t IoTApplication::localTime()
{
    /*
    TimeChangeRule *tcr;
    return _timeZone.toLocal(now(), &tcr);
    */
   return 0;
}

uint8_t IoTApplication::getRSSIasQuality(int8_t RSSI)
{
    uint8_t quality = 0;

    if (RSSI <= -100)
    {
        quality = 0;
    }
    else if (RSSI >= -50)
    {
        quality = 100;
    }
    else
    {
        quality = 2 * (RSSI + 100);
    }
    return quality;
}

bool IoTApplication::handleStaticFileRequest(AsyncWebServerRequest *request)
{
    return false;
}

/**
 * @brief Method to handle system query, return true if handled
 */
bool IoTApplication::handleCustomSystemQuery(AsyncWebServerRequest *request)
{
    String jsonStr;

    if (request->hasArg("dx"))
    {
        String dx = request->arg("dx");
        if(dx == "fwinfo")
        {
            auto pDevProp = _pIoTDevice->deviceProperties();
            jsonStr = JSONUtils::EncloseArray(
                JSONUtils::NameValueRow(F("Firmware version"), pDevProp.versionString, true) +
                JSONUtils::NameValueRow(F("Chip name"), pDevProp.chipName) +
                JSONUtils::NameValueRow(F("Device name"), pDevProp.deviceName) +
                JSONUtils::NameValueRow(F("Model"), pDevProp.deviceModel) +
                JSONUtils::NameValueRow(F("Manutacturer"), pDevProp.manufacturer) +
                JSONUtils::NameValueRow(F("Hardware ID"), pDevProp.hardwareId));
        }
        else if(dx == "hwid")
        {
            auto pDevProp = _pIoTDevice->deviceProperties();
            jsonStr = JSONUtils::EncloseObject(
                JSONUtils::Pair(F("hwid"), pDevProp.hardwareId, true));
        }
        else if(dx=="wifi")
        {
            WiFiSettings wifi1, wifi2("WIFI2");
            wifi1.read();
            wifi2.read();

            jsonStr = JSONUtils::EncloseObject(
                JSONUtils::Pair(F("ssid1"), wifi1.SSID(), true) +
                JSONUtils::Pair(F("pwd1"),  wifi1.password()) +
                JSONUtils::Pair(F("staticip1"),     wifi1.staticIP()) +
                JSONUtils::Pair(F("staticgw1"),     wifi1.staticGateway()) +
                JSONUtils::Pair(F("staticsubnet1"), wifi1.staticSubnet()) +
                JSONUtils::Pair(F("ssid2"), wifi2.SSID()) +
                JSONUtils::Pair(F("pwd2"),  wifi2.password()) +
                JSONUtils::Pair(F("staticip2"),     wifi2.staticIP()) +
                JSONUtils::Pair(F("staticgw2"),     wifi2.staticGateway()) +
                JSONUtils::Pair(F("staticsubnet2"), wifi2.staticSubnet()));
        }
    #ifdef WM_SUPPORT_HOME_ASSISTANT
        else if(dx=="mqtt")
        {
            MQTTSettings mqttSettings;
            mqttSettings.read();

            jsonStr = JSONUtils::EncloseObject(
                JSONUtils::Pair(F("host"), mqttSettings.MQTTServer(), true) +
                JSONUtils::Pair(F("port"), mqttSettings.MQTTPort()) +
                JSONUtils::Pair(F("user"), mqttSettings.MQTTUser()) +
                JSONUtils::Pair(F("pwd"),  mqttSettings.MQTTPassword()));
        }
    #endif // WM_SUPPORT_HOME_ASSISTANT
        else if(dx=="appsettings")
        {
            jsonStr = JSONUtils::EncloseObject(
                JSONUtils::NameValueRow(F("temp_unit"),
                    _appSettings.temperatureInCelsius() ? F("C") : F("F"), true));
        }
    #ifdef WM_SUPPORT_HOME_ASSISTANT
        else if(dx=="hwstatus")
        {
            jsonStr = _pIoTDevice->allComponentsStatusJSON();
        }
    #endif
    }

    if (jsonStr.isEmpty())
    {
        return false; // Not handled
    }

    ESPAsync_WiFiManagerUtils::responseApplJson(request, jsonStr);
    return true; // Handled
}

/*
char IoTApplication::getWifiStatusCharacter() const
{
    char c = DA_WIFI_NOT_CONNECTED;
    if (WiFi.isConnected())
    {
        uint8_t rssiQuality = getRSSIasQuality(WiFi.RSSI());
        if (rssiQuality >= 66)
        {
            c = DA_WIFI_SIGNAL_EXCELLENT;
        }
        else if (rssiQuality >= 33)
        {
            c = DA_WIFI_SIGNAL_GOOD;
        }
        else
        {
            c = DA_WIFI_SIGNAL_BAD;
        }
    }

    return c;
}
*/
