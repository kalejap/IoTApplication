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
#include "ESPAsync_WiFiManager.h"

#include "IoTApplication.h"
#include "WifiSettings.h"
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

const char gIOTVersionTag[] PROGMEM = MAGIC_PREFIX SW_VERSION_STRING;


IoTApplication::IoTApplication(IoTDevice* pIoTDevice) :
    _pIoTDevice(pIoTDevice),
    _webServer(80),
    //_wm(&_webServer, &_dnsServer),
    _automaticUpdateTimer(15*1000),
    _wifiUpdateTimer(60*1000)
    //_timeZone(pIoTDevice->deviceProperties().dstStart, pIoTDevice->deviceProperties().stdStart),
    //_mqtt(_wifiClient, pIoTDevice->device())
{
    _pWiFiManager = new ESPAsync_WiFiManager(&_webServer, &_dnsServer);
    _verTag = gIOTVersionTag;
}

IoTApplication::~IoTApplication()
{
    delete _pWiFiManager;
}

void IoTApplication::setup()
{
    Serial.begin(115200);

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

    delay(2000);

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

        IPAddress ipAddress;
        if (ipAddress.fromString(wifiSettings.MQTTServer().c_str()))
        {
            // MQTT broker connection using IP address
            //_mqtt.begin(ipAddress, wifiSettings.MQTTPort(), 
            //    wifiSettings.MQTTUser().c_str(), wifiSettings.MQTTPassword().c_str());
            IOTLOGINFO("MQTT connecting using IP address");
        }
        else
        {
            // MQTT broker connection using server name
            //_mqtt.begin(wifiSettings.MQTTServer().c_str(), wifiSettings.MQTTPort(), 
            //    wifiSettings.MQTTUser().c_str(), wifiSettings.MQTTPassword().c_str());
            IOTLOGINFO("MQTT connecting using server name");
        }

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

        _pWiFiManager->handleSTA();

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

    IOTLOGINFO2(F("MQTT server: |"), wifiSettings.MQTTServer(), F("|"));
    IOTLOGINFO2(F("MQTT port: |"), wifiSettings.MQTTPort(), F("|"));
    IOTLOGINFO2(F("MQTT user: |"), wifiSettings.MQTTUser(), F("|"));
    IOTLOGINFO2(F("MQTT password: |"), wifiSettings.MQTTPassword(), F("|"));

    IOTLOGINFO1(F("_ESPASYNC_WIFIMGR_LOGLEVEL_: "), _ESPASYNC_WIFIMGR_LOGLEVEL_);

    delay(2000);
}

void IoTApplication::loop()
{
    _pWiFiManager->loop();
    _pIoTDevice->preLoop();

    if (_bUsingWiFi)
    {
        //_mqtt.loop();
    }
    _pIoTDevice->postLoop();
}

void IoTApplication::update(bool bForceUpdate)
{
    if (!bForceUpdate && !_automaticUpdateTimer.elapsed())
    {
        if (_bPublishDeviceToHA)
        {
            _bPublishDeviceToHA = false;
            //publishDeviceToHA();
        }
        return;
    }

    if (bForceUpdate)
    {
        _automaticUpdateTimer.restart();
    }

    if (_bUsingWiFi)
    {
        _bPublishDeviceToHA = true;
    }
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

#ifdef WM_SUPPORT_MQTT
    ESPAsync_WMParameter customMQTTserver("mqtt_server", "MQTT server", wifiSettings.MQTTServer().c_str(), 40);
    ESPAsync_WMParameter customMQTTport("mqtt_port", "MQTT port", String(wifiSettings.MQTTPort()).c_str(), 40);
    ESPAsync_WMParameter customMQTTuser("mqtt_user", "MQTT user", wifiSettings.MQTTUser().c_str(), 40);
    ESPAsync_WMParameter customMQTTpwd("mqtt_pwd", "MQTT password", wifiSettings.MQTTPassword().c_str(), 40);
    _pWiFiManager->addParameter(&customMQTTserver);
    _pWiFiManager->addParameter(&customMQTTport);
    _pWiFiManager->addParameter(&customMQTTuser);
    _pWiFiManager->addParameter(&customMQTTpwd);
#else
    // MQTT parameters are skipped because WM_SUPPORT_MQTT is not defined
#endif

#ifdef ESP8266
    String ssid = "ESP_" + String(ESP.getChipId());
#else   //ESP32
    String ssid = "ESP_" + String(ESP_getChipId());
#endif

    ssid.toUpperCase();

    _pIoTDevice->onConfig(ssid);

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

    #ifdef WM_SUPPORT_MQTT
        wifiSettings.setMQTTServer(customMQTTserver.getValue());
        wifiSettings.setMQTTPort(String(customMQTTport.getValue()).toInt());
        wifiSettings.setMQTTUser(customMQTTuser.getValue());
        wifiSettings.setMQTTPassword(customMQTTpwd.getValue());
    #else
        // MQTT parameters are skipped because WM_SUPPORT_MQTT is not defined
    #endif

        if (wifiSettings.isDirty())
        {
            wifiSettings.save();
            IOTLOGDEBUG(F("WiFi configuration saved."));
        }

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
