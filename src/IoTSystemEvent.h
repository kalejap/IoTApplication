/*
  IoTSystemEvent.h - System event descriptor passed to IoTDevice::onSystemEvent().
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

#pragma once

#include <stdint.h>

/**
 * @brief Compact descriptor for a framework system event.
 *
 * Passed by const reference to IoTDevice::onSystemEvent().
 * Unused fields default to 0 / false; only populate fields relevant to
 * the event type.
 *
 * Field mapping by type:
 *   OTA_PROGRESS  : arg1 = bytes uploaded so far, arg2 = total bytes
 *   OTA_END       : flag = true if success, false if error
 *   WIFI_CONNECTED: arg1 = IP address as uint32_t (host byte order)
 */
struct IoTSystemEvent
{
    enum class Type : uint8_t
    {
        OTA_START,
        OTA_PROGRESS,
        OTA_END,
        WIFI_CONNECTED,
        WIFI_DISCONNECTED,
        MQTT_CONNECTED,
        MQTT_DISCONNECTED,
    };

    Type     type;
    uint32_t arg1 = 0;
    uint32_t arg2 = 0;
    bool     flag  = false;
};
