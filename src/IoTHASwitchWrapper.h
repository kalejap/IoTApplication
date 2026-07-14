
/*
  IoTHASwitchWrapper.h - HA wrapper for a GPIO-driven on/off switch (LED or relay).
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

#ifndef IOTHASWITCHWRAPPER_H
#define IOTHASWITCHWRAPPER_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "IoTHADeviceWrapperBase.h"
#include "DeviceDefines.h"   // IOT_MAX_COMPONENTS + LanguageSupport.h → L_GENERAL_ON/OFF
#include "JSONUtils.h"

/**
 * @class IoTHASwitchWrapper
 * @brief HA wrapper for a GPIO-driven on/off switch (LED or relay).
 *
 * Wraps ArduinoHA's HASwitch entity and drives a single GPIO output.
 * State changes commanded by Home Assistant over MQTT are applied to the pin
 * automatically; an optional user callback can handle additional side-effects
 * (e.g. updating an LCD or logging).
 *
 * Usage pattern:
 *   1. Construct with a GPIO pin, unique HA entity ID, and optional active-high flag.
 *   2. Register with IoTDevice::registerComponent() — begin() is called automatically
 *      by IoTDevice::preSetup().
 *   3. Configure the HA entity (setName, setIcon, etc.) in postSetup().
 *   4. Optionally call onCommand(cb) to hook additional logic.
 *   5. publishAllComponents() in the loop re-publishes the current state to MQTT.
 *
 * Example:
 * @code
 *   IoTHASwitchWrapper m_relay(D1, "relay_heat");
 *
 *   // In constructor:
 *   registerComponent(m_relay);
 *
 *   // In postSetup():
 *   m_relay.setName("Heating relay");
 *   m_relay.setIcon("mdi:radiator");
 * @endcode
 */
class IoTHASwitchWrapper : public IoTHADeviceWrapperBase
{
public:
    /**
     * @brief Callback type invoked after each state change.
     * @param state   New switch state (true = ON).
     * @param sender  Pointer to the wrapper that changed state.
     */
    using CommandCallback = void(*)(bool state, IoTHASwitchWrapper* sender);

    /**
     * @brief Construct the switch wrapper.
     *
     * @param pin        GPIO output pin (e.g. D1).
     * @param uid        Unique HA entity ID string.
     * @param activeHigh If true (default), pin HIGH = switch ON.
     *                   If false, pin LOW = switch ON (active-low relay).
     */
    IoTHASwitchWrapper(uint8_t pin, const char* uid, bool activeHigh = true)
        : _switch(uid)
        , _pin(pin)
        , _activeHigh(activeHigh)
        , _uid(uid)
    {
        if (s_instanceCount < IOT_MAX_COMPONENTS)
            s_instances[s_instanceCount++] = this;
        _switch.onCommand(onSwitchCommand);
    }

    IoTHASwitchWrapper(const IoTHASwitchWrapper&)            = delete;
    IoTHASwitchWrapper& operator=(const IoTHASwitchWrapper&) = delete;
    IoTHASwitchWrapper(IoTHASwitchWrapper&&)                 = delete;
    IoTHASwitchWrapper& operator=(IoTHASwitchWrapper&&)      = delete;

    // -----------------------------------------------------------------------
    // HA entity configuration — delegate to HASwitch
    // -----------------------------------------------------------------------

    /** @brief Set the display name of the switch in Home Assistant. */
    void setName(const char* name)
    {
        _name = name;
        _switch.setName(name);
    }

    /** @brief Return the name set via setName(), or nullptr if unset. */
    const char* name() const { return _name; }

    /** @brief Set the MaterialDesignIcons icon (e.g. "mdi:lightbulb"). */
    void setIcon(const char* icon) { _switch.setIcon(icon); }

    /**
     * @brief Set the HA device class.
     * See https://www.home-assistant.io/integrations/switch/#device-class
     */
    void setDeviceClass(const char* deviceClass) { _switch.setDeviceClass(deviceClass); }

    /** @brief Set the MQTT retain flag for HA commands. */
    void setRetain(bool retain) { _switch.setRetain(retain); }

    /**
     * @brief Enable/disable optimistic mode.
     * In optimistic mode HA updates the UI immediately without waiting for
     * a state confirmation from the device.
     */
    void setOptimistic(bool optimistic) { _switch.setOptimistic(optimistic); }

    /** @brief Return the current switch state (true = ON). */
    bool getCurrentState() const { return _switch.getCurrentState(); }

    /**
     * @brief Register an optional callback invoked after each state change.
     *
     * The callback is fired after the GPIO has been driven and the new state
     * has been confirmed back to Home Assistant.
     *
     * @param callback  Function pointer of type void(bool, IoTHASwitchWrapper*).
     */
    void onCommand(CommandCallback callback) { _callback = callback; }

    bool handleWebCommand(const char* uid, bool state) override
    {
        if (strcmp(uid, _uid) != 0)
        {
            return false;
        }
        handleCommand(state);
        return true;
    }

    // -----------------------------------------------------------------------
    // IoTHADeviceWrapperBase
    // -----------------------------------------------------------------------

    /**
     * @brief Re-publish the current switch state to MQTT.
     *        Called by IoTDevice::publishAllComponents() on each cycle.
     *
     * @param force  Pass true to publish even if the state has not changed.
     * @return true if the MQTT message was sent successfully.
     */
    bool publishValue(const bool force = false) override
    {
        return _switch.setState(_switch.getCurrentState(), force);
    }

    /**
     * @brief Return a JSON object for the web status table.
     *
     * Produces: {"name":"...","value":"On"} or {"value":"Off"}
     * The "value" string is language-dependent (L_GENERAL_ON / L_GENERAL_OFF).
     * "unit" is omitted — the web table JS handles a missing unit gracefully.
     */
    String statusJSON() const override
    {
        const char* nm = _name;
        bool hasName = nm && nm[0] != '\0';
        String obj;
        if (hasName)
        {
            obj += JSONUtils::Pair(F("name"), nm, true);
        }
        obj += JSONUtils::Pair(F("value"),
                               _switch.getCurrentState() ? L_GENERAL_ON : L_GENERAL_OFF,
                               !hasName);
        obj += JSONUtils::Pair(F("type"), F("switch"));
        obj += JSONUtils::Pair(F("uid"), _uid);
        obj += String(F(",\"state\":")) + (_switch.getCurrentState() ? F("true") : F("false"));
        return JSONUtils::EncloseObject(obj);
    }

protected:
    /**
     * @brief Initialise the GPIO pin and set the switch to OFF.
     *        Called automatically by IoTDevice::preSetup().
     */
    void begin() override
    {
        pinMode(_pin, OUTPUT);
        applyState(false);
        _switch.setCurrentState(false);
    }

private:
    void applyState(bool state)
    {
        digitalWrite(_pin, (state == _activeHigh) ? HIGH : LOW);
    }

    void handleCommand(bool state)
    {
        applyState(state);
        _switch.setCurrentState(state);   // update regardless of MQTT connection
        _switch.setState(state, true);    // attempt MQTT publish (force bypasses equality check)
        if (_callback)
            _callback(state, this);
    }

    static void onSwitchCommand(bool state, HASwitch* sender)
    {
        for (uint8_t i = 0; i < s_instanceCount; ++i)
        {
            if (&s_instances[i]->_switch == sender)
            {
                s_instances[i]->handleCommand(state);
                return;
            }
        }
    }

    HASwitch        _switch;
    uint8_t         _pin;
    bool            _activeHigh;
    const char*     _uid;
    const char*     _name     = nullptr;
    CommandCallback _callback = nullptr;

    // Static registry — bridges the plain HASwitch callback to the owning wrapper.
    // Sized by IOT_MAX_COMPONENTS (defined in DeviceDefines.h, default 8).
    inline static IoTHASwitchWrapper* s_instances[IOT_MAX_COMPONENTS] = {};
    inline static uint8_t             s_instanceCount                  = 0;
};

#endif // IOTHASWITCHWRAPPER_H
