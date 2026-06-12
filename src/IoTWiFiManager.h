/*
  IIoTDevice.h - Abstract class that represents an IoT device.
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

#ifndef IOTWIFIMANAGER_H
#define IOTWIFIMANAGER_H

#include <ESPAsync_WiFiManager.h>

class IoTApplication;

class IoTWiFiManager : public ESPAsync_WiFiManager  
{
public:
    IoTWiFiManager(AsyncWebServer * webserver, AsyncDNSServer *dnsserver, const char *iHostname = "");
    IoTWiFiManager(AsyncWebServer * webserver, const char * username = "", const char * password = "",
        const char *iHostname = "");

    void setApplication(IoTApplication* pApp)
    {
        _pApp = pApp;
    }

    void attachCustomHandlers(ArRequestFilterFunction filtern) override;

    bool handleCustomSystemQuery(AsyncWebServerRequest *request) override;
private:
    IoTApplication* _pApp = nullptr;
};

#endif // IOTWIFIMANAGER_H
