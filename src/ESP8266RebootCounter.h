/*
  ESP8266RebootCounter.h - Persistent per-reason reboot counter for ESP8266.
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
#ifdef WM_SUPPORT_HOME_ASSISTANT

#include <Preferences.h>
#include "IoTHADeviceWrapperBase.h"
#include "IoTDebug.h"
#include "JSONUtils.h"

/**
 * @class ESP8266RebootCounter
 * @brief Tracks one NVS-backed counter per ESP8266 reset reason (0–6).
 *
 * Register with IoTDevice::registerComponent() — begin() fires during
 * preSetup(), reads the reset reason, increments the matching counter, and
 * saves it back to NVS.  The counts appear automatically in the hwstatus
 * JSON endpoint via statusJSON().
 *
 * Reset reason mapping:
 *   0 – REASON_DEFAULT_RST      → PowerOn
 *   1 – REASON_WDT_RST          → WatchdogHW
 *   2 – REASON_EXCEPTION_RST    → Exception
 *   3 – REASON_SOFT_WDT_RST     → WatchdogSW
 *   4 – REASON_SOFT_RESTART     → SoftRestart
 *   5 – REASON_DEEP_SLEEP_AWAKE → DeepSleep
 *   6 – REASON_EXT_SYS_RST     → ExtReset
 */
class ESP8266RebootCounter : public IoTHADeviceWrapperBase
{
public:
    static constexpr uint8_t REASON_COUNT = 7;

    /** Return the stored count for a specific reset reason (0–6). */
    uint32_t count(uint8_t reason) const
    {
        return (reason < REASON_COUNT) ? _count[reason] : 0;
    }

    /** Sum of WDT HW + Exception + WDT SW counts. */
    uint32_t crashCount()       const { return _count[1] + _count[2] + _count[3]; }

    /** Sum of SoftRestart + ExtReset counts. */
    uint32_t userRestartCount() const { return _count[4] + _count[6]; }

    /** Cold-boot / power-outage count (REASON_DEFAULT_RST). */
    uint32_t powerOnCount()     const { return _count[0]; }

    /** Reset reason code recorded on this boot (0–6, or 0xFF if not yet initialised). */
    uint8_t  lastReason()       const { return _lastReason; }

    bool publishValue(const bool /*force*/ = false) override { return false; }

    String statusJSON() const override
    {
        static const char* const labels[REASON_COUNT] = {
            "PowerOn", "WatchdogHW", "Exception", "WatchdogSW",
            "SoftRestart", "DeepSleep", "ExtReset"
        };
        String s;
        for (uint8_t i = 0; i < REASON_COUNT; ++i)
        {
            if (i > 0) s += ',';
            String obj;
            obj += JSONUtils::Pair(F("name"),  labels[i], true);
            obj += JSONUtils::Pair(F("value"), (unsigned int)_count[i]);
            s += JSONUtils::EncloseObject(obj);
        }
        return s;
    }

protected:
    void begin() override
    {
        Preferences pref;
        pref.begin("REBOOT", false);

        char key[3] = "r0";
        for (uint8_t i = 0; i < REASON_COUNT; ++i)
        {
            key[1] = static_cast<char>('0' + i);
            _count[i] = pref.getUInt(key, 0);
        }

        _lastReason = static_cast<uint8_t>(ESP.getResetInfoPtr()->reason);
        if (_lastReason < REASON_COUNT)
        {
            key[1] = static_cast<char>('0' + _lastReason);
            pref.putUInt(key, ++_count[_lastReason]);
        }
        pref.end();

        IOTLOGINFO3(F("Reboot reason="), _lastReason, F(" crash="), crashCount());
        IOTLOGINFO1(F("Reboot user="), userRestartCount());
        IOTLOGINFO1(F("Reboot powerOn="), powerOnCount());
    }

private:
    uint32_t _count[REASON_COUNT] = {};
    uint8_t  _lastReason          = 0xFF;
};

#endif // WM_SUPPORT_HOME_ASSISTANT
