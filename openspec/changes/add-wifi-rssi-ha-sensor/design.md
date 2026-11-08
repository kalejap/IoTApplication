## Context

`IoTDevice` holds a list of `IoTHADeviceWrapperBase*` components registered via `registerComponent()`. Each component participates in the `begin()` / `update()` / `publishValue()` lifecycle driven by `IoTApplication`. `IoTHASensorNumberWrapper<T>` is the existing pattern for numeric HA sensors — it wraps `HASensorNumber`, handles name/icon/unit setters, and implements `registerWithMqtt()`.

`ESP8266RebootCounter` is the reference for a framework-level diagnostic component in this library: defined in its own header, guarded by `#ifdef WM_SUPPORT_HOME_ASSISTANT`, auto-constructed as a member of a device but registered manually. For RSSI we go one step further — auto-registration in `IoTDevice` itself (Option B from the exploration).

## Goals / Non-Goals

**Goals:**
- Zero application code changes — existing device subclasses gain the sensor automatically.
- RSSI marks itself unavailable in HA when WiFi is down, rather than publishing stale values.
- Language files in IoTApplication use the same selector pattern as IoTDS18B20 and ESPAsyncWiFiManager.

**Non-Goals:**
- Reboot counter HA entities (separate future change).
- Configurable update interval (uses the existing 15 s `_automaticUpdateTimer`).
- Quality percentage conversion (raw dBm only; HA has built-in signal-strength cards).

## Decisions

### 1 — Extend `IoTHASensorNumberWrapper<int>` rather than own a `HASensorNumber` directly

`IoTHASensorNumberWrapper<int>` already implements `registerWithMqtt()`, `publishValue()`, `accept()`, `setName()`, `setIcon()`, and the full HA metadata API. Subclassing it costs nothing and avoids duplicating that logic.

*Alternative considered*: compose `HASensorNumber` as a member (like `PumpStateController`) — rejected because that would re-implement setters already in the wrapper.

### 2 — `_wifiRssiSensor` as a member of `IoTDevice`, auto-registered in constructor

Placing it in `IoTDevice` makes it available to every device without any per-project wiring. The constructor calls `registerComponent(_wifiRssiSensor)` under `#ifdef WM_SUPPORT_HOME_ASSISTANT`.

*Alternative considered*: opt-in instantiation in device subclasses (like `m_rebootCounter` in SFBC) — rejected per the user's decision to use Option B for zero-boilerplate.

### 3 — Availability via `HASensorNumber::setAvailability(true)` + `publishAvailability(bool)`

ArduinoHA supports per-entity availability: call `setAvailability(true)` once in `begin()` to include the availability topic in discovery, then call `publishAvailability(false/true)` in `update()` based on `WiFi.status()`.

The alternative (skipping publish when disconnected and letting the value go stale) was rejected — HA would keep showing the last reading with no indication it is outdated.

### 4 — New language files in IoTApplication (`iot_en_US.h` / `iot_sk_SK.h` / `IoTLanguageSupport.h`)

Matches the established pattern in IoTDS18B20 and ESPAsyncWiFiManager. The selector header checks `LANGUAGE_SK_SK` first, falls back to English. Both files define `L_WIFI_RSSI_SENSOR_NAME` as `"WiFi RSSI"`.

*Alternative*: hardcode the name in `begin()` — rejected because it breaks the localization convention and prevents future translations.

### 5 — `statusJSON()` returns a single `{"name": ..., "value": ...}` object

Consistent with how `ESP8266RebootCounter::statusJSON()` formats its entries for the hwstatus web panel. If WiFi is disconnected the value published is the last valid RSSI (or 0 at boot before any reading).

## Risks / Trade-offs

- **`IoTDevice.h` gains a new header include** (`ESP8266WifiRssiSensor.h` → `ESP8266WiFi.h`). Every translation unit that includes `IoTDevice.h` will transitively pull in the WiFi header. This is acceptable because the library already targets ESP8266 WiFi applications exclusively.
- **ArduinoHA `publishAvailability` call in `update()`** adds one extra MQTT publish when the WiFi state transitions. This is negligible.
- **`WiFi.RSSI() == 0` guard** prevents publishing a spurious zero at boot before association. The `_currentValue` starts at `0` (default-initialized int); if the first update fires before WiFi connects, the entity will be marked unavailable and `0` is never published.
