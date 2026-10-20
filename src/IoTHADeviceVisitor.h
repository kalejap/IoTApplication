
/*
  IoTHADeviceVisitor.h - Visitor interface for Home Assistant device wrappers.
  Copyright (c) 2026 Peter Kaleja.  All right reserved.

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

#ifndef IOTHADEVICEVISITOR_H
#define IOTHADEVICEVISITOR_H

// Forward declarations — breaks circular includes; pointer/ref params only need incomplete types.
template<typename T>
class IoTHASensorNumberWrapper;
class IoTHASwitchWrapper;

/**
 * @class IoTHADeviceVisitor
 * @brief Abstract visitor for Home Assistant device wrappers.
 *
 * Implement this interface to inspect or operate on a heterogeneous component
 * list (IoTHADeviceWrapperBase pointers) without downcasting at the call site.
 *
 * Composite wrappers (e.g. IoTBME280Sensor, IoTDS18B20MultiSensor) drill into
 * their individual IoTHASensorNumberWrapper<float> slots — no overload for
 * composite types is needed.
 *
 * Example:
 * @code
 *   class MyVisitor : public IoTHADeviceVisitor {
 *   public:
 *       void Visit(IoTHASensorNumberWrapper<float>* s) override {
 *           Serial.println(s->name());
 *       }
 *       void Visit(IoTHASwitchWrapper* sw) override {
 *           Serial.println(sw->getCurrentState() ? "ON" : "OFF");
 *       }
 *   };
 *
 *   MyVisitor v;
 *   device.acceptAllComponents(v);
 * @endcode
 */
class IoTHADeviceVisitor
{
public:
    IoTHADeviceVisitor()          = default;
    virtual ~IoTHADeviceVisitor() = default;

    /** @brief Called for each numeric sensor wrapper (or each slot of a composite). */
    virtual void Visit(IoTHASensorNumberWrapper<float>* sensor) = 0;

    /** @brief Called for each GPIO switch wrapper. */
    virtual void Visit(IoTHASwitchWrapper* sw) = 0;
};

#endif // IOTHADEVICEVISITOR_H
