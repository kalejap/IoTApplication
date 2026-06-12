/*
  IoTWiFiManager.cpp - Implementation of WiFi manager for IoT devices
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


#include "IoTWiFiManager.h"
#include "IoTApplication.h"
#include "ESPAsync_WiFiManagerUtils.h"

IoTWiFiManager::IoTWiFiManager(AsyncWebServer * webserver, AsyncDNSServer *dnsserver, const char *iHostname)
    : ESPAsync_WiFiManager(webserver, dnsserver, iHostname)
{
}

IoTWiFiManager::IoTWiFiManager(AsyncWebServer * webserver, const char * username, const char * password,
    const char *iHostname)
    : ESPAsync_WiFiManager(webserver, username, password, iHostname)
{
}

void IoTWiFiManager::attachCustomHandlers(ArRequestFilterFunction filtern)
{
    // Do supermessaging
    ESPAsync_WiFiManager::attachCustomHandlers(filtern);

    if (_pApp != nullptr)
    {
        //_pApp
    }
    // Add custom handlers here if needed
}

bool IoTWiFiManager::handleCustomSystemQuery(AsyncWebServerRequest *request)
{
    if (_pApp == nullptr)
    {
        return false; // Not handled
    }

    return _pApp->handleCustomSystemQuery(request);
}
