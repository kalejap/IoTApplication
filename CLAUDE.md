# CLAUDE: IoTApplication Library

## Library overview

- **Name:** IoTApplication
- **Author:** Peter Kaleja
- **Platform:** Arduino / PlatformIO (ESP8266 primary target, architectures=*)
- **Purpose:** Framework base classes for ESP8266 IoT devices with WiFi, MQTT, Home Assistant integration, OTA update, NTP time sync, I²C LCD display, and async web server.

## What this library does

- Provides `IoTApplication` — the main lifecycle manager (WiFi, MQTT, OTA, web server, NTP).
- Provides `IoTDevice` — the abstract application base class; subclasses add sensors, switches, and display pages.
- Provides `IoTHADeviceWrapperBase` and concrete wrappers (`IoTHASwitchWrapper`, `IoTHASensorNumberWrapper`) for registering Home Assistant entities.
- Provides `IoTTextDisplay` + `IoTDisplayPage` for cycling LCD pages without flicker.
- Provides `IoTSystemEvent` — a compact event descriptor passed to `onSystemEvent()` for OTA, WiFi, MQTT, and restart lifecycle events.
- Exposes an async web UI with `/hw-status.js`, `/api/switch`, and extensible custom buttons via virtual hooks.

## Key source files

| File | Purpose |
|---|---|
| `src/IoTApplication.h/.cpp` | Main lifecycle class — WiFi/MQTT/OTA/web server/NTP |
| `src/IoTDevice.h/.cpp` | Abstract device base — components, display pages, web dispatch |
| `src/IoTHADeviceWrapperBase.h` | Abstract base for all HA entity wrappers |
| `src/IoTHASwitchWrapper.h` | GPIO-driven on/off switch, web + MQTT controllable |
| `src/IoTHASensorNumberWrapper.h` | Template wrapper for numeric HA sensors |
| `src/IoTHACompositeDeviceWrapperBase.h` | Abstract base for multi-entity HA wrappers |
| `src/IoTTextDisplay.h` | Abstract interface for row×col character displays |
| `src/IoTDisplayPage.h` | Abstract display page with `render()` and `durationMs()` |
| `src/IoTSystemEvent.h` | POD event descriptor for lifecycle events |
| `src/IoTWiFiManager.h` | Subclass of `ESPAsync_WiFiManager`; bridges to `IoTApplication` |
| `src/DeviceDefines.h` | `IOT_MAX_COMPONENTS`, `MAX_DISPLAY_PAGES`, language macros |
| `src/JSONUtils.h` | Helpers for building JSON fragments |
| `src/LanguageSupport.h` | Localized string constants (e.g. `L_GENERAL_ON`, `L_GENERAL_OFF`) |

## Architecture

### Lifecycle

```
main.cpp
  └── IoTApplication::setup()
        ├── IoTDevice::preSetup()      — HA device init, component begin()
        ├── (WiFi, MQTT, OTA, web)
        └── IoTDevice::postSetup()     — user overrides configure HA entities

  └── IoTApplication::loop()
        ├── IoTDevice::preLoop()       — user sensor reads
        ├── updateAllComponents()      — poll hardware
        ├── publishAllComponents()     — push to MQTT
        ├── IoTDevice::postLoop()      — calls tickDisplayPages()
        └── IoTDevice::postLoopEnd()   — user cleanup
```

### HA component registration

Concrete `IoTDevice` subclasses own component instances and register them in their constructor:

```cpp
class MyDevice : public IoTDevice {
    IoTHASwitchWrapper _relay{D1, "relay_heat"};
public:
    MyDevice() { registerComponent(_relay); }
};
```

`registerComponent()` stores a pointer in `_components[]` (max `MAX_COMPONENTS`).
`IoTDevice::preSetup()` calls `begin()` on every component.
`updateAllComponents()` / `publishAllComponents()` iterate all components each loop cycle.

### Display page cycling

```cpp
// Register pages in constructor
registerPage(myPage);

// tickDisplayPages() is called from postLoop() automatically.
// Freeze/unfreeze the display during OTA / restart via onSystemEvent().
```

Pages implement `IoTDisplayPage` with `render(IoTTextDisplay&)` and `durationMs()`.

### Web command dispatch

Web UI sends `GET /api/switch?id=<uid>&state=<0|1>`.
`IoTApplication` calls `IoTDevice::dispatchWebCommand(uid, state)`, which iterates components and calls `handleWebCommand()` on each until one returns `true`.

## Virtual hooks on IoTDevice

| Method | When to override |
|---|---|
| `postSetup()` | Configure HA entity names, icons, callbacks after WiFi is up |
| `preLoop()` | Read sensors before `updateAllComponents()` |
| `postLoop()` | Extra work after publish; base calls `tickDisplayPages()` |
| `postLoopEnd()` | Cleanup at end of loop iteration |
| `onSystemEvent()` | React to OTA/WiFi/MQTT/restart events; base freezes display |
| `onCustomIndexButtons()` | Return PROGMEM HTML for extra buttons on the main web page |
| `onCustomSettingsButtons()` | Return PROGMEM HTML for extra buttons on the settings page |
| `onSaveConfigParameters()` | Persist custom NVS settings when web form is submitted |

## Compile-time flags

| Flag | Effect |
|---|---|
| `WM_SUPPORT_HOME_ASSISTANT` | Enables HADevice, HAMqtt, component registry, `/api/switch` |
| `_IOT_REAL_TIME` | Enables NTPClient, `setupTime()`, `IoTTextDisplay::printDateTime()` |
| `WM_REMOTE_UPDATE` | Enables remote OTA via JSON manifest |
| `LANGUAGE_EN_US` / `LANGUAGE_CS_CZ` | Selects localised string set |

## Settings persistence

`IoTApplication` exposes NVS-backed settings via a `Settings` base class with `read()`, `save()`, and `isDirty()`. Concrete devices subclass it and call `onSaveConfigParameters()` when the web form posts.

## IoTSystemEvent types

| Type | Extra fields |
|---|---|
| `OTA_START` | — |
| `OTA_PROGRESS` | `arg1` = bytes so far, `arg2` = total bytes |
| `OTA_END` | `flag` = true if success |
| `WIFI_CONNECTED` | `arg1` = IP as `uint32_t` |
| `WIFI_DISCONNECTED` | — |
| `MQTT_CONNECTED` | — |
| `MQTT_DISCONNECTED` | — |
| `RESTARTING` | — |

## How to add a new HA component type

1. Create a class that inherits `IoTHADeviceWrapperBase`.
2. Implement `publishValue(bool force)` — push state to HA via ArduinoHA entity.
3. Optionally override `begin()`, `update()`, `statusJSON()`, `handleWebCommand()`.
4. Construct the instance in your `IoTDevice` subclass and call `registerComponent()`.

## Editing guidance for Claude

- Prefer changes in library `src/` over modifying `.pio/libdeps/` cached copies.
- All virtual hooks default to no-ops; only override what you need.
- `IOT_MAX_COMPONENTS` (default 8) is defined in `DeviceDefines.h` — increase there if more components are needed.
- `IoTHASwitchWrapper` stores a static instance registry to bridge the plain `HASwitch` callback. The registry is also bounded by `IOT_MAX_COMPONENTS`.
- Do not add `Version:` or version-specific fields to this file; version is tracked in `library.properties`.

## Code styling

Follow Arduino library conventions:

- Always use curly brackets for `if` / `else`, never single-line branches:

```cpp
if (a == 0)
{
    doSomething();
}

if (str.empty())
{
    logMessage("String is empty");
}
else
{
    postMessage(str);
}
```

- Prefer `const char*` PROGMEM strings for display text; use `F()` macro for string literals in flash.
- Keep ISR and callback functions as short as possible; delegate work to the component's member function.
