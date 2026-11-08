/*
  ESP8266WifiRssiSensor.h - Home Assistant WiFi RSSI sensor for ESP8266.
  Copyright (c) 2026 Peter Kaleja.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/

#pragma once
#ifdef WM_SUPPORT_HOME_ASSISTANT

#include <ESP8266WiFi.h>
#include "IoTHASensorNumberWrapper.h"
#include "IoTLanguageSupport.h"
#include "JSONUtils.h"

class ESP8266WifiRssiSensor : public IoTHASensorNumberWrapper<int>
{
public:
    ESP8266WifiRssiSensor() : IoTHASensorNumberWrapper<int>("wifi_rssi") {}

protected:
    void begin() override
    {
        _sensor.setAvailability(true);
        setDeviceClass("signal_strength");
        setUnitOfMeasurement("dBm");
        setStateClass("measurement");
        setIcon("mdi:wifi");
        setName(L_WIFI_RSSI_SENSOR_NAME);
    }

    bool update(bool /*force*/ = false) override
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            _sensor.setAvailability(false);
            return true;
        }
        int rssi = WiFi.RSSI();
        if (rssi == 0) return true;
        setCurrentValue(rssi);
        _sensor.setAvailability(true);
        return true;
    }

public:
    String statusJSON() const override
    {
        String obj;
        obj += JSONUtils::Pair(F("name"),  L_WIFI_RSSI_SENSOR_NAME, true);
        obj += JSONUtils::Pair(F("value"), _currentValue);
        return JSONUtils::EncloseObject(obj);
    }
};

#endif // WM_SUPPORT_HOME_ASSISTANT
