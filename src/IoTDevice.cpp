/*
  IoTDevice.cpp - Abstract class that represent a device or
  Home assistant device.
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

#include "IoTApplication.h"
#include "IoTDevice.h"
#include "IoTDebug.h"

IoTDevice::IoTDevice(const DeviceProperties& properties) :
    _properties(properties)
{}

IoTDevice::~IoTDevice()
{}

void IoTDevice::preSetup()
{
#ifdef WM_SUPPORT_HOME_ASSISTANT
    byte mac[6];
    WiFi.macAddress(mac);
    _device.setUniqueId(mac, sizeof(mac));

    // DeviceProperties strings are in PROGMEM — copy to static RAM buffers for the HA library.
    static char s_name[32], s_model[32], s_manufacturer[32], s_version[16];
    strncpy_P(s_name,         _properties.deviceName,    sizeof(s_name)         - 1);
    strncpy_P(s_model,        _properties.deviceModel,   sizeof(s_model)        - 1);
    strncpy_P(s_manufacturer, _properties.manufacturer,  sizeof(s_manufacturer) - 1);
    strncpy_P(s_version,      _properties.versionString, sizeof(s_version)      - 1);

    _device.setName(s_name);
    _device.setModel(s_model);
    _device.setSoftwareVersion(s_version);
    _device.setAvailability(true);
    _device.setManufacturer(s_manufacturer);

    forEachComponent(&IoTHADeviceWrapperBase::begin);
#endif
}

#ifdef WM_SUPPORT_HOME_ASSISTANT

void IoTDevice::registerComponent(IoTHADeviceWrapperBase& component)
{
    if (_componentCount < MAX_COMPONENTS)
        _components[_componentCount++] = &component;
    else
        IOTLOGWARN(F("IoTDevice: MAX_COMPONENTS reached, component not registered"));
}

void IoTDevice::updateAllComponents(bool force)
{
    forEachComponent(&IoTHADeviceWrapperBase::update, force);
}

void IoTDevice::publishAllComponents(bool force)
{
    forEachComponent(&IoTHADeviceWrapperBase::publishValue, force);
}

String IoTDevice::allComponentsStatusJSON() const
{
    String result(F("["));
    bool first = true;
    for (uint8_t i = 0; i < _componentCount; ++i)
    {
        String part = _components[i]->statusJSON();
        if (part.isEmpty()) continue;
        if (!first) result += ',';
        result += part;
        first = false;
    }
    result += F("]");
    return result;
}

#endif // WM_SUPPORT_HOME_ASSISTANT

void IoTDevice::registerPage(IoTDisplayPage& page)
{
    if (_displayPageCount < MAX_DISPLAY_PAGES)
        _displayPages[_displayPageCount++] = &page;
    else
        IOTLOGWARN(F("IoTDevice: MAX_DISPLAY_PAGES reached, page not registered"));
}

void IoTDevice::onUpdateDisplay(IoTTextDisplay& display)
{
    if (_displayPageCount == 0) return;
    if (_displayPageTimer.elapsed(_displayPages[_currentPageIndex]->durationMs()))
        _currentPageIndex = (_currentPageIndex + 1) % _displayPageCount;
    _displayPages[_currentPageIndex]->render(display);
}

void IoTDevice::_tickDisplay()
{
    if (!_pDisplay || _displayFrozen) return;
    if (!_displayTimerReady)
    {
        _displayPageTimer.restart();
        _displayTimerReady = true;
    }
    onUpdateDisplay(*_pDisplay);
}

