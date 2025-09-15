
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

#ifndef IOTHASENSORNUMBERWRAPPER_H
#define IOTHASENSORNUMBERWRAPPER_H

#include "IoTHADeviceWrapperBase.h"



/**
 * @brief Wrapper class for Home Assistant number sensor integration.
 *
 * This template class provides a convenient wrapper for managing a Home Assistant (HA) number sensor
 * using the HASensorNumber class. It allows you to set and publish numeric values (integer or floating point)
 * to Home Assistant, with support for precision and feature configuration.
 *
 * @tparam T The numeric type of the sensor value (e.g., int, float, double).
 */
template<typename T>
class IoTHASensorNumberWrapper : public IoTHADeviceWrapperBase
{
public:
    /**
     * @brief Construct a new IoTHASensorNumberWrapper object.
     *
     * @param uniqueId The unique ID of the sensor. It must be unique within your device.
     * @param precision The precision of the floating point number displayed in the HA panel (default: PrecisionP0).
     * @param features Bitmask of features to enable for the sensor (default: DefaultFeatures).
     */
    IoTHASensorNumberWrapper(
        const char* uniqueId,
        const HABaseDeviceType::NumberPrecision precision = HABaseDeviceType::PrecisionP0,
        const uint16_t features = HASensor::DefaultFeatures
    );

    /**
     * @brief Publish the current value to Home Assistant.
     *
     * @param force If true, force publishing even if the value hasn't changed (default: false).
     * @return true if the value was published successfully, false otherwise.
     */
    bool publishValue(const bool force = false) override
    {
        return _sensor.setValue(_currentValue, force);
    }

    /**
     * @brief Set the current value of the sensor.
     *
     * @param value The new value to set.
     */
    void setCurrentValue(T value)
    {
        _currentValue = value;
    }

    void setName(const char* name)
    {
        _name = name;
        _sensor.setName(name);
    }

    /**
     * Sets class of the device.
     * You can find list of available values here: https://www.home-assistant.io/integrations/sensor/#device-class
     *
     * @param deviceClass The class name.
     */
    void setDeviceClass(const char* deviceClass)
    {
        _sensor.setDeviceClass(deviceClass);
    }

    /**
     * Sets class of the state for the long term stats.
     * See: https://developers.home-assistant.io/docs/core/entity/sensor/#long-term-statistics
     *
     * @param stateClass The class name.
     */
    void setStateClass(const char* stateClass)
    {
        _sensor.setStateClass(stateClass);
    }

    /**
     * Forces HA panel to process each incoming value (MQTT message).
     * It's useful if you want to have meaningful value graphs in history.
     *
     * @param forceUpdate
     */
    void setForceUpdate(bool forceUpdate)
    {
        _sensor.setForceUpdate(forceUpdate);
    }

    /**
     * Sets icon of the sensor.
     * Any icon from MaterialDesignIcons.com (for example: `mdi:home`).
     *
     * @param class The icon name.
     */
    void setIcon(const char* icon)
    {
        _sensor.setIcon(icon);
    }

    /**
     * Defines the units of measurement of the sensor, if any.
     *
     * @param units For example: °C, %
     */
    void setUnitOfMeasurement(const char* unitOfMeasurement)
    { 
        _unitOfMeasurement = unitOfMeasurement;
        _sensor.setUnitOfMeasurement(unitOfMeasurement);
    }

protected:
    /**
     * @brief The underlying Home Assistant number sensor object.
     */
    HASensorNumber _sensor;

    /**
     * @brief The current value of the sensor.
     */
    T _currentValue;

    /**
     * @brief The name that was set using setName method. It can be nullptr.
     */
    const char* _name = nullptr;

    /**
     * @brief Represents the unit of measurement for the sensor value.
     * 
     * This character indicates the unit (e.g., 'C' for Celsius, 'F' for Fahrenheit, etc.)
     * associated with the sensor's readings.
     */
    const char _unitOfMeasurement = nullptr;
};

#endif // IOTHASENSORNUMBERWRAPPER_H