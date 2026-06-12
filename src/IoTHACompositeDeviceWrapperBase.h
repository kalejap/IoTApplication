
/*
  IoTHACompositeDeviceWrapperBase.h - Abstract base for composite Home Assistant devices.
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

#ifndef IOTHACOMPOSITEDEVICEWRAPPERBASE_H
#define IOTHACOMPOSITEDEVICEWRAPPERBASE_H

#include "IoTHADeviceWrapperBase.h"

/**
 * @class IoTHACompositeDeviceWrapperBase
 * @brief Abstract base for Home Assistant wrappers that expose multiple HA entities.
 *
 * A composite device is one physical component (e.g. a multi-sensor IC or a motor driver)
 * that maps to several distinct Home Assistant entities. Extending
 * IoTHADeviceWrapperBase makes composite wrappers usable wherever single-entity
 * wrappers are expected, so one polymorphic list can hold both.
 *
 * Concrete composite devices derive from IoTHACompositeDeviceWrapper<Wrappers...>,
 * which provides compile-time indexed access to the individual HA entity wrappers
 * via get<I>() and a default publishValue() that publishes all of them.
 *
 * Derived classes may override publishValue() when only a subset of the entities
 * should be published (e.g. a DS18B20 bus that found fewer sensors than its maximum).
 */
class IoTHACompositeDeviceWrapperBase : public IoTHADeviceWrapperBase
{
public:
    IoTHACompositeDeviceWrapperBase() = default;
    virtual ~IoTHACompositeDeviceWrapperBase() = default;

    // publishValue() is inherited as pure virtual from IoTHADeviceWrapperBase.
};

#endif // IOTHACOMPOSITEDEVICEWRAPPERBASE_H
