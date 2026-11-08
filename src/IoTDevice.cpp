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
#include "IoTHADeviceVisitor.h"
#include "IoTDebug.h"

IoTDevice::IoTDevice(const DeviceProperties& properties) :
    _properties(properties)
{
#ifdef WM_SUPPORT_HOME_ASSISTANT
    registerComponent(_wifiRssiSensor);
#endif
}

IoTDevice::~IoTDevice()
{}

void IoTDevice::postSetup()
{
#ifdef WM_SUPPORT_HOME_ASSISTANT
    forEachComponent(&IoTHADeviceWrapperBase::begin);

    // Prime all components so the display shows real values on the first tick
    // instead of "------" for up to 15 seconds (the automatic update timer period).
    updateAllComponents(true);
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

bool IoTDevice::dispatchWebCommand(const char* uid, bool state)
{
    for (uint8_t i = 0; i < _componentCount; ++i)
    {
        if (_components[i]->handleWebCommand(uid, state))
        {
            return true;
        }
    }
    return false;
}

void IoTDevice::acceptAllComponents(IoTHADeviceVisitor& visitor)
{
    for (uint8_t i = 0; i < _componentCount; ++i)
    {
        _components[i]->accept(visitor);
    }
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
    {
        _currentPageIndex = (_currentPageIndex + 1) % _displayPageCount;
        yield();  // let WiFi SDK run before the I2C bit-bang block
        _displayPages[_currentPageIndex]->render(display);
    }
}

void IoTDevice::tickDisplayPages()
{
    if (!_pDisplay || _displayFrozen || _displayPageCount == 0)
    {
        return;
    }
    if (!_displayTimerReady)
    {
        _displayPageTimer.restart();
        _displayTimerReady = true;
        yield();  // let WiFi SDK run before the I2C bit-bang block
        _displayPages[_currentPageIndex]->render(*_pDisplay);
        return;
    }
    onUpdateDisplay(*_pDisplay);
}
