
/*
  IoTHADeviceWrapperBase.h - Abstract class for Home Assistant device.
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

#ifndef IOTHADEVICEWRAPPERBASE_H
#define IOTHADEVICEWRAPPERBASE_H

#include <ArduinoHA.h>

// Forward declaration — allows IoTDevice to be a friend without a full include.
class IoTDevice;

/**
 * @class IoTHADeviceWrapperBase
 * @brief Abstract base class for Home Assistant device wrappers.
 *
 * This class defines a common interface for device wrappers that integrate with Home Assistant (HA).
 * It enforces the implementation of a method to publish the device's value or state to HA.
 *
 * All derived classes must implement the publishValue() method, which is intended to send the current
 * value or state of the device to Home Assistant, optionally forcing the update even if the value has not changed.
 */
class IoTHADeviceWrapperBase
{
    friend class IoTDevice;

public:
    /**
     * @brief Default constructor.
     *
     * Constructs a new IoTHADeviceWrapperBase object.
     */
    IoTHADeviceWrapperBase() = default;

    /**
     * @brief Virtual destructor.
     *
     * Ensures proper cleanup of derived classes.
     */
    virtual ~IoTHADeviceWrapperBase() = default;

    /**
     * @brief Publish the current value or state to Home Assistant.
     *
     * This pure virtual function must be implemented by all derived classes. It is responsible for
     * sending the current value or state of the device to Home Assistant.
     *
     * @param force If true, force publishing even if the value hasn't changed (default: false).
     * @return true if the value was published successfully, false otherwise.
     */
    virtual bool publishValue(const bool force = false) = 0;

    /**
     * @brief Read hardware and refresh internal state.
     *
     * Default is a no-op returning true. Override in concrete classes that need to
     * poll hardware (e.g. requestTemperatures() for DS18B20) before publishValue().
     *
     * @param force If true, force update even if state hasn't changed.
     * @return true on success, false on hardware error.
     */
    virtual bool update(bool force = false) { return true; }

    /**
     * @brief Return a JSON fragment describing current sensor state for the web UI.
     *
     * Returns the *inner content* of the component's array contribution: a
     * comma-separated sequence of {"key":value,...} objects WITHOUT surrounding [].
     * IoTDevice::allComponentsStatusJSON() wraps the combined result in one [...].
     *
     * Default returns an empty String (component contributes nothing to status).
     */
    virtual String statusJSON() const { return String(); }

protected:
    /**
     * @brief Initialise the device/sensor on application start-up.
     *
     * Called once by IoTDevice::preSetup() for every registered component.
     * Default is a no-op. Override in concrete classes that require hardware
     * initialisation (e.g. OneWire bus scan, sensor resolution setup).
     */
    virtual void begin() {}
};

#endif // IOTHADEVICEWRAPPERBASE_H