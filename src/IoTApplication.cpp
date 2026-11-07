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
    unsigned long ota_progress_millis = 0;
}

IoTApplication::IoTApplication(IoTDevice* pIoTDevice) :
    _pIoTDevice(pIoTDevice),
    _webServer(80),
    //_wm(&_webServer, &_dnsServer),
    _automaticUpdateTimer(15*1000),
    _wifiUpdateTimer(60*1000)
    //_timeZone(pIoTDevice->deviceProperties().dstStart, pIoTDevice->deviceProperties().stdStart),
#ifdef _IOT_REAL_TIME
    ,_ntpClient(_ntpUDP, "europe.pool.ntp.org", 0, 300000UL)
#endif
#ifdef WM_SUPPORT_HOME_ASSISTANT
    ,_mqtt(_wifiClient, _haDevice)
#endif
{
    _pWiFiManager = new IoTWiFiManager(&_webServer, &_dnsServer);
    _pWiFiManager->setApplication(this);
    _pWiFiManager->setHardwareId(pIoTDevice->deviceProperties().hardwareId);
#ifdef WM_SUPPORT_HOME_ASSISTANT
    // HAMqtt (_mqtt) is now constructed — register all pre-existing HA entities.
    // Without this, entities constructed before IoTApplication (global objects) are
    // silently skipped by HABaseDeviceType because HAMqtt::instance() was null.
    _pIoTDevice->registerAllWithMqtt();
#endif
}

IoTApplication::~IoTApplication()
{
    delete _pWiFiManager;
}

void IoTApplication::registerCommonRoutes()
{
    _webServer.on("/hw-status.js", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send_P(200, "application/javascript", WM_PK_HW_STATUS_JS);
    });
#ifdef WM_SUPPORT_HOME_ASSISTANT
    _webServer.on("/api/switch", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!request->hasArg("id") || !request->hasArg("state"))
        {
            ESPAsync_WiFiManagerUtils::responseApplJson(
                request, String(F("{\"ok\":false,\"error\":\"missing args\"}")));
            return;
        }
        const String& uid = request->arg("id");
        bool state = (request->arg("state") == "1");
        bool ok = _pIoTDevice->dispatchWebCommand(uid.c_str(), state);
        ESPAsync_WiFiManagerUtils::responseApplJson(
            request, String(F("{\"ok\":")) + (ok ? F("true") : F("false")) + F("}"));
    });
#endif
}

void IoTApplication::registerRootHandler(ArRequestFilterFunction filter)
{
    PGM_P customButtons = _pIoTDevice->onCustomIndexButtons();
    _webServer.on("/", HTTP_GET, [customButtons](AsyncWebServerRequest* req) {
        IOTLOGINFO(F("GET / handler fired"));
        ESPAsync_WiFiManagerUtils::responseText(req,
            ESPAsync_WiFiManagerUtils::getCustomIndexPage(customButtons));
    }).setFilter(filter);
}

void IoTApplication::registerSystemEventCallbacks()
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
    _pWiFiManager->onPreReboot([this]() {
        _pIoTDevice->onSystemEvent({IoTSystemEvent::Type::RESTARTING});
    });

#ifdef ESP8266
    _wifiGotIPHandler = WiFi.onStationModeGotIP(
        [this](const WiFiEventStationModeGotIP& ev) {
            WiFi.setSleep(false);
            IoTSystemEvent e;
            e.type = IoTSystemEvent::Type::WIFI_GOT_IP;
            e.arg1 = (uint32_t)ev.ip;
            _pIoTDevice->onSystemEvent(e);
#ifdef _IOT_REAL_TIME
            _bNeedsTimeSync = true;
#endif
        });
    _wifiConnectHandler = WiFi.onStationModeConnected(
        [this](const WiFiEventStationModeConnected&) {
            _pIoTDevice->onSystemEvent({IoTSystemEvent::Type::WIFI_CONNECTED});
        });
    _wifiDisconnectHandler = WiFi.onStationModeDisconnected(
        [this](const WiFiEventStationModeDisconnected& ev) {
           IoTSystemEvent e;
            e.type = IoTSystemEvent::Type::WIFI_DISCONNECTED;
            e.arg1 = (uint32_t)ev.reason;
            _pIoTDevice->onSystemEvent(e);
        });
#endif
}

void IoTApplication::setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println(F("IoTApplication::setup()"));

    _appSettings.read();
    IOTLOGINFO1(F("Temperature unit: "), _appSettings.temperatureInCelsius() ? F("Celsius") : F("Fahrenheit"));
#ifdef WM_GEO_LOCATION
    _geoSettings.read();
#endif

    _pIoTDevice->preSetup();
 
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
    if (wifiSettings.SSID()[0] == '\0' || _pIoTDevice->isConfigTriggeredOnStartUp())
    {
        configure();
    }

    if (wifiSettings.SSID()[0] != '\0')
    {
        IOTLOGDEBUG1(F("SSID: "), wifiSettings.SSID());
        IOTLOGDEBUG1(F("Password: "), wifiSettings.password());
        //WiFi.hostname("_IoTApplicationTest1");
        //WiFi.setPhyMode(WIFI_PHY_MODE_11N);
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiSettings.SSID(), wifiSettings.password());

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
            WiFi.setSleep(false);
            IOTLOGDEBUG(F(". Connected"));
            IOTLOGDEBUG1(F("IP:"), WiFi.localIP());
        }
        else
        {
            IOTLOGDEBUG(F(". Not connected"));
        }

        delay(1000);

        _bUsingWiFi = true;

        _pIoTDevice->onAddConfigRoutes(_webServer);
        registerCommonRoutes();
        registerRootHandler(ON_STA_FILTER);
        registerSystemEventCallbacks();

        _pWiFiManager->setCustomIndexButtons(_pIoTDevice->onCustomIndexButtons());
        _pWiFiManager->setCustomSettingsButtons(_pIoTDevice->onCustomSettingsButtons());
        _pWiFiManager->handleSTA();

        _pWiFiManager->setSaveConfigCallback([this]() {
            WiFiSettings ws1("WIFI");
            ws1.read();
            ws1.setSSID(_pWiFiManager->getSSID());
            ws1.setPassword(_pWiFiManager->getPW());
            if (ws1.isDirty())
                ws1.save();

            WiFiSettings ws2("WIFI2");
            ws2.read();
            ws2.setSSID(_pWiFiManager->getSSID1());
            ws2.setPassword(_pWiFiManager->getPW1());
            if (ws2.isDirty())
                ws2.save();
        });

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

#ifdef WM_GEO_LOCATION
        _webServer.on("/geo", HTTP_POST, [this](AsyncWebServerRequest *request) {
            if (request->hasArg("lat"))
            {
                _geoSettings.setLatitude(request->arg("lat").toFloat());
            }
            if (request->hasArg("lng"))
            {
                _geoSettings.setLongitude(request->arg("lng").toFloat());
            }
            if (request->hasArg("alt"))
            {
                _geoSettings.setAltitude(request->arg("alt").toFloat());
            }
            if (_geoSettings.isDirty())
            {
                _geoSettings.save();
            }
            request->redirect("/settings");
        });
#endif

        _webServer.begin();
    }

    // Setup time
    setupTime();

    delay(500);

#ifdef WM_SUPPORT_HOME_ASSISTANT
    setupHA();
#endif

    _pIoTDevice->postSetup();

 }

void IoTApplication::loop()
{
    _pWiFiManager->loop();
    _pIoTDevice->preLoop();

#ifdef WM_SUPPORT_HOME_ASSISTANT
    if (_bUsingWiFi && WiFi.status() == WL_CONNECTED)
    {
        _mqtt.loop();
    }
#endif

    update();

    _pIoTDevice->postLoop();
}

void IoTApplication::update(bool bForceUpdate)
{
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

#ifdef _IOT_REAL_TIME
    if (_bUsingWiFi)
    {
        bool synced = false;
        if (_bNeedsTimeSync)
        {
            synced = _ntpClient.forceUpdate();
        }
        else if (_ntpClient.isTimeSet())
        {
            synced = _ntpClient.update();
        }
        if (synced)
        {
            _bNeedsTimeSync = false;
            Timezone tz(deviceProperties().dstStart, deviceProperties().stdStart);
            setTime(tz.toLocal((time_t)_ntpClient.getEpochTime()));
        }
    }
#endif

#ifdef WM_SUPPORT_HOME_ASSISTANT
    _pIoTDevice->updateAllComponents(bForceUpdate);
    if (_bUsingWiFi)
        _pIoTDevice->publishAllComponents(bForceUpdate);
#endif
}

#ifdef WM_SUPPORT_HOME_ASSISTANT

void IoTApplication::setupHA()
{
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    _haDevice.setUniqueId(mac, sizeof(mac));

    // DeviceProperties strings are in PROGMEM — copy to static RAM buffers for the HA library.
    static char s_name[32], s_model[32], s_manufacturer[32], s_version[16];
    strncpy_P(s_name,         _pIoTDevice->deviceProperties().deviceName,    sizeof(s_name)         - 1);
    strncpy_P(s_model,        _pIoTDevice->deviceProperties().deviceModel,   sizeof(s_model)        - 1);
    strncpy_P(s_manufacturer, _pIoTDevice->deviceProperties().manufacturer,  sizeof(s_manufacturer) - 1);
    strncpy_P(s_version,      _pIoTDevice->deviceProperties().versionString, sizeof(s_version)      - 1);

    _haDevice.setName(s_name);
    _haDevice.setModel(s_model);
    _haDevice.setSoftwareVersion(s_version);
    _haDevice.setAvailability(true);
    _haDevice.setManufacturer(s_manufacturer);

    _wifiClient.setTimeout(2);  // 2-second TCP connect timeout — prevents blocking loop on unreachable broker

    MQTTSettings mqttSettings;
    mqttSettings.read();

    // PubSubClient stores char* pointers, not copies — buffers must outlive setupHA().
    static char s_mqttServer[MQTTSettings::SERVER_MAX_LEN + 1];
    static char s_mqttUser[MQTTSettings::USER_MAX_LEN + 1];
    static char s_mqttPassword[MQTTSettings::PWD_MAX_LEN + 1];
    strncpy(s_mqttServer,   mqttSettings.MQTTServer(),   sizeof(s_mqttServer)   - 1);
    strncpy(s_mqttUser,     mqttSettings.MQTTUser(),     sizeof(s_mqttUser)     - 1);
    strncpy(s_mqttPassword, mqttSettings.MQTTPassword(), sizeof(s_mqttPassword) - 1);
    s_mqttServer[sizeof(s_mqttServer)   - 1] = '\0';
    s_mqttUser[sizeof(s_mqttUser)       - 1] = '\0';
    s_mqttPassword[sizeof(s_mqttPassword) - 1] = '\0';

    if (s_mqttServer[0] != '\0')
    {
        IPAddress ipAddress;
        if (ipAddress.fromString(s_mqttServer))
        {
            // Already an IP address — connect directly, no DNS needed.
            _mqtt.begin(ipAddress, mqttSettings.MQTTPort(), s_mqttUser, s_mqttPassword);
            IOTLOGINFO("MQTT: connecting using IP address");
        }
        else if (WiFi.status() == WL_CONNECTED && WiFi.hostByName(s_mqttServer, ipAddress))
        {
            // Resolved hostname to IP at startup — PubSubClient will reconnect via IP,
            // avoiding synchronous DNS lookups in loop() that can block for ~15 s.
            _mqtt.begin(ipAddress, mqttSettings.MQTTPort(), s_mqttUser, s_mqttPassword);
            IOTLOGINFO("MQTT: hostname resolved to IP at startup");
        }
        else
        {
            // WiFi not up or DNS failed — fall back to hostname.
            _mqtt.begin(s_mqttServer, mqttSettings.MQTTPort(), s_mqttUser, s_mqttPassword);
            IOTLOGINFO("MQTT: connecting using hostname (DNS will run in loop)");
        }
    }

    IOTLOGINFO2(F("MQTT server: |"), s_mqttServer, F("|"));
    IOTLOGINFO2(F("MQTT port: |"), mqttSettings.MQTTPort(), F("|"));
    IOTLOGINFO2(F("MQTT user: |"), s_mqttUser, F("|"));
    IOTLOGINFO2(F("MQTT password: |"), s_mqttPassword, F("|"));

    IOTLOGINFO1(F("_ESPASYNC_WIFIMGR_LOGLEVEL_: "), _ESPASYNC_WIFIMGR_LOGLEVEL_);
}
#endif

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
    char mqttPortStr[6];
    snprintf(mqttPortStr, sizeof(mqttPortStr), "%u", mqttSettings.MQTTPort());
    ESPAsync_WMParameter customMQTTserver("mqtt_server", "MQTT server", mqttSettings.MQTTServer(), 40);
    ESPAsync_WMParameter customMQTTport("mqtt_port", "MQTT port", mqttPortStr, 40);
    ESPAsync_WMParameter customMQTTuser("mqtt_user", "MQTT user", mqttSettings.MQTTUser(), 40);
    ESPAsync_WMParameter customMQTTpwd("mqtt_pwd", "MQTT password", mqttSettings.MQTTPassword(), 40);
    _pWiFiManager->addParameter(&customMQTTserver);
    _pWiFiManager->addParameter(&customMQTTport);
    _pWiFiManager->addParameter(&customMQTTuser);
    _pWiFiManager->addParameter(&customMQTTpwd);
#endif

    // Allow the device to register extra HTTP routes (e.g. live JSON endpoints, static assets).
    _pIoTDevice->onAddConfigRoutes(_webServer);
    registerCommonRoutes();
    registerRootHandler(ON_AP_FILTER);

    // Inject optional <head> element (e.g. <script src> + <style>) from the device.
    const char* headElem = _pIoTDevice->onCustomHeadElement();
    if (headElem) _pWiFiManager->setCustomHeadElement(headElem);

    // Pass PROGMEM custom button pointers for the index and settings pages.
    _pWiFiManager->setCustomIndexButtons(_pIoTDevice->onCustomIndexButtons());
    _pWiFiManager->setCustomSettingsButtons(_pIoTDevice->onCustomSettingsButtons());

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
    registerSystemEventCallbacks();

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
#ifdef _IOT_REAL_TIME
    _ntpClient.begin();
    // Defer the first sync to the update() cycle rather than blocking here.
    // DNS and routing are often not stable immediately after association, so
    // forceUpdate() reliably times out at boot and wastes ~1.5 s in setup().
    _bNeedsTimeSync = true;
#endif
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
#ifdef _IOT_REAL_TIME
    return now();
#else
    return 0;
#endif
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
    #ifdef WM_GEO_LOCATION
        else if(dx=="geo")
        {
            jsonStr = JSONUtils::EncloseObject(
                JSONUtils::Pair(F("lat"), _geoSettings.latitude(), true) +
                JSONUtils::Pair(F("lng"), _geoSettings.longitude()) +
                JSONUtils::Pair(F("alt"), _geoSettings.altitude()));
        }
    #endif // WM_GEO_LOCATION
    #ifdef WM_SUPPORT_HOME_ASSISTANT
        else if(dx=="hwstatus")
        {
            jsonStr = _pIoTDevice->allComponentsStatusJSON();
        }
    #endif
        else
        {
            String customJson;
            if (_pIoTDevice->onCustomStatusQuery(dx, customJson))
                jsonStr = customJson;
        }
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
