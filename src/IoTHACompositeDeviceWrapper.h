
/*
  IoTHACompositeDeviceWrapper.h - Template base for composite Home Assistant devices.
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

#ifndef IOTHACOMPOSITEDEVICEWRAPPER_H
#define IOTHACOMPOSITEDEVICEWRAPPER_H

#include <tuple>
#include <utility>
#include "IoTHACompositeDeviceWrapperBase.h"

/**
 * @class IoTHACompositeDeviceWrapper
 * @brief Template base for composite HA devices backed by a heterogeneous tuple of wrappers.
 *
 * Works like std::tuple: the Wrappers... parameter list defines the exact HA entity types
 * held by this composite device. Each wrapper is constructed in-place with one positional
 * argument passed to the IoTHACompositeDeviceWrapper constructor.
 *
 * Example — a BME280 exposing temperature, humidity and pressure:
 * @code
 * class IoTBME280Sensor : public IoTHACompositeDeviceWrapper<
 *     IoTHASensorNumberWrapper<float>,   // temperature
 *     IoTHASensorNumberWrapper<float>,   // humidity
 *     IoTHASensorNumberWrapper<float>>   // pressure
 * {
 * public:
 *     IoTBME280Sensor(uint8_t addr,
 *                     const char* uidTemp,
 *                     const char* uidHum,
 *                     const char* uidPres)
 *         : IoTHACompositeDeviceWrapper<...>(uidTemp, uidHum, uidPres)
 *         , _address(addr) {}
 * };
 * @endcode
 *
 * Individual wrappers are accessed at compile time via get<I>():
 * @code
 *     sensor.get<0>().setName("Temperature");
 *     sensor.get<1>().setName("Humidity");
 * @endcode
 *
 * The default publishValue() publishes every wrapper. Override it in the concrete class
 * to publish only a subset (e.g. sensors found at runtime on a 1-Wire bus).
 *
 * @tparam Wrappers  HA entity wrapper types (e.g. IoTHASensorNumberWrapper<float>).
 *                   Each type must be constructible from a single const char* uniqueId.
 */
template<typename... Wrappers>
class IoTHACompositeDeviceWrapper : public IoTHACompositeDeviceWrapperBase
{
public:
    /** Number of HA entities in this composite device. */
    static constexpr size_t SIZE = sizeof...(Wrappers);

    /**
     * @brief Construct the composite wrapper.
     *
     * Each argument is forwarded to the corresponding wrapper's constructor in order,
     * matching the Wrappers... parameter list. Typically each argument is a
     * const char* uniqueId, matching the IoTHASensorNumberWrapper(uniqueId) constructor.
     *
     * The wrappers are constructed in-place inside _wrappers (no copy or move).
     *
     * @param args  One construction argument per wrapper type, in order.
     */
    template<typename... CtorArgs>
    explicit IoTHACompositeDeviceWrapper(CtorArgs&&... args)
        : _wrappers(std::forward<CtorArgs>(args)...)
    {
        static_assert(sizeof...(CtorArgs) == sizeof...(Wrappers),
            "IoTHACompositeDeviceWrapper: number of constructor arguments "
            "must match the number of wrapper types.");
    }

    /**
     * @brief Return a reference to the I-th HA entity wrapper (0-based).
     *
     * I must be a compile-time constant less than SIZE.
     */
    template<size_t I>
    typename std::tuple_element<I, std::tuple<Wrappers...>>::type& get()
    {
        static_assert(I < SIZE, "IoTHACompositeDeviceWrapper::get<I>: index out of range.");
        return std::get<I>(_wrappers);
    }

    /** @brief Const overload of get<I>(). */
    template<size_t I>
    const typename std::tuple_element<I, std::tuple<Wrappers...>>::type& get() const
    {
        static_assert(I < SIZE, "IoTHACompositeDeviceWrapper::get<I>: index out of range.");
        return std::get<I>(_wrappers);
    }

    /**
     * @brief Publish all HA entity wrappers to Home Assistant.
     *
     * Calls publishValue(force) on every wrapper in Wrappers... order.
     * Returns true only if every wrapper published successfully.
     *
     * Override in concrete classes to restrict publishing to a subset of wrappers
     * (e.g. only the sensors physically present on the bus).
     *
     * @param force  Pass true to publish even if the value has not changed.
     */
    bool publishValue(const bool force = false) override
    {
        return publishAll(force, std::index_sequence_for<Wrappers...>{});
    }

protected:
    /** Storage for all HA entity wrappers. Accessible to derived classes. */
    std::tuple<Wrappers...> _wrappers;

private:
    template<size_t... Is>
    bool publishAll(const bool force, std::index_sequence<Is...>)
    {
        bool allOk = true;
        // Fold expression — evaluated left to right, short-circuit disabled intentionally
        // so every wrapper gets a publish attempt.
        ((allOk &= std::get<Is>(_wrappers).publishValue(force)), ...);
        return allOk;
    }
};

// ---------------------------------------------------------------------------
// NCompositeWrapper<T, N>
//
// Helper that generates IoTHACompositeDeviceWrapper<T, T, ..., T> with exactly
// N copies of type T, using std::index_sequence so the pack expansion can
// repeat a single type without listing it N times manually.
//
// Usage:
//   typename NCompositeWrapper<IoTHASensorNumberWrapper<float>, 2>::type
//   → IoTHACompositeDeviceWrapper<IoTHASensorNumberWrapper<float>,
//                                  IoTHASensorNumberWrapper<float>>
// ---------------------------------------------------------------------------
template<typename T, size_t N, typename Seq = std::make_index_sequence<N>>
struct NCompositeWrapper;

template<typename T, size_t N, size_t... Is>
struct NCompositeWrapper<T, N, std::index_sequence<Is...>>
{
    // AlwaysT ignores its size_t argument and always yields T.
    // Expanding AlwaysT<Is>... therefore produces N copies of T.
    template<size_t> using AlwaysT = T;
    using type = IoTHACompositeDeviceWrapper<AlwaysT<Is>...>;
};

#endif // IOTHACOMPOSITEDEVICEWRAPPER_H
