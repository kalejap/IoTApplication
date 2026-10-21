# Getting Started with IoTApplication

This guide walks you through creating a new application on top of the
IoTApplication framework, from a bare PlatformIO project to a fully-featured
WiFi-connected IoT device.

**Prerequisites assumed:** you are already familiar with PlatformIO and the
Arduino framework. This guide covers only the framework-specific parts.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Minimal application](#2-minimal-application)
3. [Adding Home Assistant / MQTT](#3-adding-home-assistant--mqtt)
4. [Adding real-time clock (NTP)](#4-adding-real-time-clock-ntp)
5. [Adding a display](#5-adding-a-display)
6. [Adding hardware sensors](#6-adding-hardware-sensors)
7. [Remote OTA updates](#7-remote-ota-updates)
8. [Geolocation settings](#8-geolocation-settings)
9. [Localisation](#9-localisation)
- [Appendix A — Full build flags reference](#appendix-a--full-build-flags-reference)

---

## 1. Prerequisites

- **PlatformIO** installed (CLI or IDE extension)
- An **ESP8266** or **ESP32** board
- The custom libraries stored locally (optional — see note below)

### Local library path

If you keep custom libraries (IoTApplication, IoTDS18B20, etc.) in a local
directory rather than pulling them from GitHub on every build, add
`lib_extra_dirs` to your `platformio.ini`:

```ini
lib_extra_dirs = G:\IoT\libraries   ; adjust path to your local library root
```

---

## 2. Minimal application

The smallest project that uses IoTApplication gives you a WiFi configuration
portal, NVS settings persistence, and an async web server — with no sensors,
no display, and no Home Assistant integration.

### platformio.ini

```ini
[env:esp12e]
platform  = espressif8266
board     = esp12e
framework = arduino

lib_ldf_mode = deep+
lib_deps =
    https://github.com/kalejap/IoTUtilities.git
    https://github.com/kalejap/ESPAsyncWiFiManager.git
    IoTApplication
    mathieucarbou/ESPAsyncWebServer
    me-no-dev/ESPAsyncTCP
    devyte/ESPAsyncDNSServer
    me-no-dev/ESPAsyncUDP
    khoih-prog/ESP_DoubleResetDetector
    vshymanskyy/Preferences

build_flags =
    -D SW_MAJOR_VERSION=1
    -D SW_MINOR_VERSION=0
    -D SW_PATCH_VERSION=0
    -D _IOT_DEVICE_NAME="MyDevice"
    -D _IOT_DEVICE_MODEL="v1"
    -D _IOT_DEVICE_MANUFACTURER="My Name"
    -D _IOT_COPYRIGHT="&copy; My Name 2026"
    -D _IOT_DEBUG_LOGLEVEL_=3
    -D LANGUAGE_EN_US
```

### Always-required libraries

| Library | Purpose |
|---|---|
| `IoTApplication` | The framework itself |
| `IoTUtilities` | `Timer`, `StringUtil` helpers used by the framework |
| `ESPAsyncWiFiManager` | WiFi config portal, web server, OTA UI |
| `ESPAsyncWebServer` | Async HTTP server |
| `ESPAsyncTCP` (ESP8266) / `AsyncTCP` (ESP32) | Async TCP layer |
| `ESPAsyncDNSServer` | Captive portal DNS redirect |
| `ESPAsyncUDP` | UDP transport (NTP) |
| `ESP_DoubleResetDetector` | Double-reset → force config portal |
| `Preferences` | NVS settings persistence |

### Identity build flags

These flags appear in the web portal, Home Assistant device info, and OTA
manifest. All are optional but strongly recommended.

| Flag | Type | Default | Description |
|---|---|---|---|
| `SW_MAJOR_VERSION` | `int` | `1` | Firmware major version |
| `SW_MINOR_VERSION` | `int` | `0` | Firmware minor version |
| `SW_PATCH_VERSION` | `int` | `0` | Firmware patch version |
| `_IOT_DEVICE_NAME` | `string` | `"device"` | Human-readable device name |
| `_IOT_DEVICE_MODEL` | `string` | `""` | Model identifier |
| `_IOT_DEVICE_MANUFACTURER` | `string` | `""` | Manufacturer name |
| `_IOT_COPYRIGHT` | `string` | `""` | Copyright text shown in portal footer |
| `_IOT_DEBUG_LOGLEVEL_` | `int` | `0` | Log verbosity: 0=off 1=error 2=warn 3=info 4=debug |

### main.cpp

```cpp
#include <IoTApplication.h>
#include <IoTDevice.h>

class MyDevice : public IoTDevice
{
public:
    MyDevice() : IoTDevice(IOT_DEVICE_PROPERTIES(dstRule, stdRule))
    {
    }

private:
    // US Eastern DST rules — adjust for your timezone
    TimeChangeRule dstRule{"EDT", Second, Sun, Mar, 2, -240};
    TimeChangeRule stdRule{"EST", First,  Sun, Nov, 2, -300};
};

class MyApp : public IoTApplication
{
public:
    MyApp(IoTDevice* dev) : IoTApplication(dev) {}
};

MyDevice theDevice;
MyApp    theApp(&theDevice);

void setup() { theApp.setup(); }
void loop()  { theApp.loop(); }
```

On first boot (or after a double-reset) the device opens an access point named
after `_IOT_DEVICE_NAME`. Connect to it and visit `192.168.4.1` to configure
WiFi credentials.

---

## 3. Adding Home Assistant / MQTT

Enable MQTT and the Home Assistant entity registry with the
`WM_SUPPORT_HOME_ASSISTANT` flag.

### Additional lib_deps

```ini
lib_deps =
    ; ... existing deps ...
    knolleary/PubSubClient
    dawidchyrzynski/home-assistant-integration
```

### Additional build flag

```ini
build_flags =
    ; ... existing flags ...
    -D WM_SUPPORT_HOME_ASSISTANT
```

The WiFi portal will show an MQTT configuration page (broker host, port,
username, password). Settings are saved to NVS automatically.

### Adding a switch (relay / GPIO output)

```cpp
#include <IoTHASwitchWrapper.h>

class MyDevice : public IoTDevice
{
public:
    MyDevice() : IoTDevice(IOT_DEVICE_PROPERTIES(dstRule, stdRule))
    {
        registerComponent(m_relay);
    }

    void postSetup() override
    {
        m_relay.setName("Heater relay");
        m_relay.setIcon("mdi:radiator");
        m_relay.onCommand([](bool state, IoTHASwitchWrapper* sw) {
            Serial.println(state ? "ON" : "OFF");
        });
    }

private:
    IoTHASwitchWrapper m_relay{D1, "relay_heat"};

    TimeChangeRule dstRule{"EDT", Second, Sun, Mar, 2, -240};
    TimeChangeRule stdRule{"EST", First,  Sun, Nov, 2, -300};
};
```

`registerComponent()` in the constructor ensures `begin()`, `update()`, and
`publishValue()` are called automatically each loop cycle.

### Adding a numeric sensor

```cpp
#include <IoTHASensorNumberWrapper.h>

class MyDevice : public IoTDevice
{
public:
    MyDevice() : IoTDevice(IOT_DEVICE_PROPERTIES(dstRule, stdRule))
    {
        registerComponent(m_temp);
    }

    void postSetup() override
    {
        m_temp.setName("Room temperature");
        m_temp.setDeviceClass("temperature");
        m_temp.setUnitOfMeasurement("°C");
        m_temp.setIcon("mdi:thermometer");
    }

    void preLoop() override
    {
        m_temp.setCurrentValue(readTemperature());
    }

private:
    IoTHASensorNumberWrapper<float> m_temp{"sensor_temp",
        HABaseDeviceType::PrecisionP1};

    float readTemperature() { return 21.5f; }  // replace with real reading

    TimeChangeRule dstRule{"EDT", Second, Sun, Mar, 2, -240};
    TimeChangeRule stdRule{"EST", First,  Sun, Nov, 2, -300};
};
```

---

## 4. Adding real-time clock (NTP)

Enable NTP synchronisation and timezone-aware local time with the
`_IOT_REAL_TIME` flag.

### Additional lib_deps

```ini
lib_deps =
    ; ... existing deps ...
    arduino-libraries/NTPClient
    jchristensen/Timezone
    paulstoffregen/Time
```

### Additional build flag

```ini
build_flags =
    ; ... existing flags ...
    -D _IOT_REAL_TIME
```

### Timezone — DST rules

Pass your timezone's DST rules to `IOT_DEVICE_PROPERTIES`. The macro expects
two `TimeChangeRule` members — summer time first, then standard time:

```cpp
// Central European Time (CET/CEST)
TimeChangeRule dstRule{"CEST", Last, Sun, Mar, 2, +120};  // UTC+2
TimeChangeRule stdRule{"CET",  Last, Sun, Oct, 3, +60};   // UTC+1
```

```cpp
// US Eastern
TimeChangeRule dstRule{"EDT", Second, Sun, Mar, 2, -240};
TimeChangeRule stdRule{"EST", First,  Sun, Nov, 2, -300};
```

The offset value is **minutes from UTC** (positive = east, negative = west).

---

## 5. Adding a display

The framework supports any character LCD through the `IoTTextDisplay` interface.

### Connecting a display

```cpp
#include <IoTLCDDisplay.h>   // PCF8574-based I2C LCD via LiquidCrystal_I2C

class MyDevice : public IoTDevice
{
public:
    MyDevice() : IoTDevice(IOT_DEVICE_PROPERTIES(dstRule, stdRule))
    {
        setDisplay(m_lcd);
    }

private:
    IoTLCDDisplay m_lcd{0x27, 16, 2};  // I2C address, cols, rows

    TimeChangeRule dstRule{"CEST", Last, Sun, Mar, 2, +120};
    TimeChangeRule stdRule{"CET",  Last, Sun, Oct, 3, +60};
};
```

### Display library options

| Hardware | Library to add | Class to use |
|---|---|---|
| PCF8574-based I2C LCD (most common) | `IoTLCDDisplay` + `marcoschwartz/LiquidCrystal_I2C` | `IoTLCDDisplay` |
| Winstar WH series I2C LCD | `LCDWinstarI2C` (local) | `IoTWinstarDisplay` |

For `IoTWinstarDisplay` include `<IoTWinstarDisplay.h>` from the `LCDWinstarI2C`
library instead of `<IoTLCDDisplay.h>`.

### Adding display pages

Define a page by subclassing `IoTDisplayPage`:

```cpp
#include <IoTDisplayPage.h>

class MyPage : public IoTDisplayPage
{
public:
    void render(IoTTextDisplay& display) override
    {
        display.printLine(0, "Hello");
        display.printLine(1, "World");
    }

    unsigned long durationMs() const override { return 3000; }  // show for 3 s
};
```

Register it in the device constructor:

```cpp
class MyDevice : public IoTDevice
{
public:
    MyDevice() : IoTDevice(IOT_DEVICE_PROPERTIES(dstRule, stdRule))
    {
        setDisplay(m_lcd);
        registerPage(m_page);
    }

private:
    IoTLCDDisplay m_lcd{0x27, 16, 2};
    MyPage        m_page;
    // ...
};
```

Pages cycle automatically. The framework pauses cycling during OTA and restart
events and writes status text to the display via `onSystemEvent()`.

---

## 6. Adding hardware sensors

### 6a. DS18B20 temperature sensor

The `IoTDS18B20` library wraps one or more DS18B20 sensors on a OneWire bus and
publishes them as Home Assistant sensor entities. It also provides a self-hosted
web configuration UI for assigning sensors to named slots.

**lib_deps additions:**

```ini
lib_deps =
    ; ... existing deps ...
    paulstoffregen/OneWire
    milesburton/DallasTemperature
    https://github.com/kalejap/IoTDS18B20.git
```

**Minimal single-sensor usage:**

```cpp
#include <IoTDS18B20SingleSensor.h>

class MyDevice : public IoTDevice
{
public:
    MyDevice() : IoTDevice(IOT_DEVICE_PROPERTIES(dstRule, stdRule))
    {
        registerComponent(m_temp);
    }

private:
    IoTDS18B20SingleSensor m_temp{D4, "temp_room"};  // OneWire pin, HA unique id
    // ...
};
```

For multi-sensor setups, NVS slot assignment, and the web config UI, see the
[IoTDS18B20 README](https://github.com/kalejap/IoTDS18B20).

### 6b. BME280 environmental sensor

The `IoTBME280` library publishes temperature, humidity, atmospheric pressure,
sea-level pressure, and dew point as five HA sensor entities.

**lib_deps additions:**

```ini
lib_deps =
    ; ... existing deps ...
    adafruit/Adafruit BME280 Library
    adafruit/Adafruit Unified Sensor
    IoTBME280
```

**Minimal usage:**

```cpp
#include <IoTBME280Sensor.h>

class MyDevice : public IoTDevice
{
public:
    MyDevice() : IoTDevice(IOT_DEVICE_PROPERTIES(dstRule, stdRule))
    {
        registerComponent(m_bme);
    }

private:
    IoTBME280Sensor m_bme{"bme280"};  // HA unique-id prefix
    // ...
};
```

---

## 7. Remote OTA updates

Remote OTA lets the device check a hosted manifest file, compare firmware
versions, and download an update — all triggered from the web portal's
Settings page without needing USB access.

### 7a. What you need

**Build flags:**

```ini
build_flags =
    ; ... existing flags ...
    -D WM_REMOTE_UPDATE
    -D _IOT_OTA_UPDATE_URL=\"https://example.com/firmware/MyDevice/manifest.json\"
```

**Post-build script** (generates the manifest and named firmware binary after
each build):

```ini
extra_scripts = post:extra_scripts/post_manifest.py
```

Copy `post_manifest.py` from the
[IoTApplicationDemo](https://github.com/kalejap/IoTApplicationDemo)
`extra_scripts/` directory into your project.

### 7b. What post_manifest.py generates

After every successful build the script produces two files in
`.pio/build/firmware/`:

```
.pio/build/firmware/
    firmware1.0.5_en-us.bin     ← language-stamped copy of firmware.bin
    manifest.json               ← OTA manifest (accumulated across language builds)
```

The binary name is `firmware<version>_<lang-tag>.bin` where `<version>` comes
from `SW_MAJOR/MINOR/PATCH_VERSION` and `<lang-tag>` from the active
`LANGUAGE_*` flag (`en-us`, `sk-sk`, …).

### 7c. manifest.json format

```json
{
  "version": "1.0.5",
  "hwid": "ESP8266|MyDevice|v1|My Name",
  "firmwares": {
    "en-us": "https://example.com/firmware/MyDevice/1.0.5/firmware1.0.5_en-us.bin"
  }
}
```

| Field | Description |
|---|---|
| `version` | Current firmware version string |
| `hwid` | `platform\|device_name\|model\|manufacturer` — used by the device to verify compatibility |
| `firmwares` | Map of language tag → absolute URL to the firmware binary |

The `firmwares` map **accumulates** across multiple language builds of the same
version. Building `LANGUAGE_EN_US` then `LANGUAGE_SK_SK` results in both
`"en-us"` and `"sk-sk"` entries in the same `manifest.json`. When the version
number changes, the map resets.

The firmware binary URL is derived automatically:
`<base_url>/<version>/<bin_name>`, where `<base_url>` is
`_IOT_OTA_UPDATE_URL` with the last path segment stripped.

### 7d. Server directory layout

Upload the generated files so the server serves:

```
<device-root>/
    manifest.json                     ← always at root (checked by device)
    1.0.5/
        firmware1.0.5_en-us.bin
        firmware1.0.5_sk-sk.bin
    1.0.4/
        firmware1.0.4_en-us.bin
```

`_IOT_OTA_UPDATE_URL` must point to the `manifest.json` at the root:

```
https://example.com/firmware/MyDevice/manifest.json
```

### 7e. Upload pipeline

The demo project includes ready-to-use upload scripts that copy the generated
files to a server via SCP:

| Script | Platform |
|---|---|
| `upload_firmware.ps1` | Windows (PowerShell) |
| `upload_firmware.bat` | Windows (batch) |
| `upload_firmware.winscp` | WinSCP scripting |

All scripts read `SW_*_VERSION` from `platformio.ini` automatically, create
the versioned subdirectory on the server, upload the firmware binaries into it,
and upload `manifest.json` to the device root.

A typical target is a Raspberry Pi (or any SSH-accessible host) running Home
Assistant. Home Assistant's `/config/www/` directory is served at
`/local/` over HTTP, making it a convenient free file server:

```
_IOT_OTA_UPDATE_URL = "https://homeassistant.local/local/firmware/MyDevice/manifest.json"
```

### 7f. Multi-language builds

To ship multiple language variants:

1. Define one `[env]` per language in `platformio.ini`:

```ini
[env:esp12e_en_us]
build_flags = ${env.build_flags} -D LANGUAGE_EN_US

[env:esp12e_sk_sk]
build_flags = ${env.build_flags} -D LANGUAGE_SK_SK
```

2. Build all environments: `pio run`
3. Each build appends its language entry to the shared `manifest.json`
4. Run the upload script once — it uploads all `firmware*_*.bin` files and the final `manifest.json`

The device portal shows a language selector in the update dialog, populated
from the `firmwares` map in the manifest.

---

## 8. Geolocation settings

Enable a geolocation settings page (latitude, longitude, altitude stored in
NVS) with the `WM_GEO_LOCATION` flag.

```ini
build_flags =
    ; ... existing flags ...
    -D WM_GEO_LOCATION
```

This adds a **Location** section to the portal's Settings page. Values are
stored in the `"GEO"` NVS namespace and are accessible at runtime via
`GeoSettings`:

```cpp
#include <GeoSettings.h>

GeoSettings geo;
geo.read();
float lat = geo.getLatitude();
float lng = geo.getLongitude();
float alt = geo.getAltitude();
```

No additional libraries are required.

---

## 9. Localisation

The framework ships with English (default) and Slovak string sets. Select one
with a build flag:

| Flag | Language |
|---|---|
| `LANGUAGE_EN_US` | English (used if neither flag is defined) |
| `LANGUAGE_SK_SK` | Slovak |

```ini
build_flags =
    ; ... existing flags ...
    -D LANGUAGE_EN_US
```

To build separate language variants as distinct PlatformIO environments:

```ini
[env]
; ... common settings ...

[env:esp12e_en_us]
build_flags = ${env.build_flags} -D LANGUAGE_EN_US

[env:esp12e_sk_sk]
build_flags = ${env.build_flags} -D LANGUAGE_SK_SK
```

Build a specific environment: `pio run -e esp12e_en_us`

---

## Appendix A — Full build flags reference

### Feature flags

| Flag | Default | Description |
|---|---|---|
| `WM_SUPPORT_HOME_ASSISTANT` | off | Enables MQTT broker connection, ArduinoHA entity registry, component `update()`/`publishValue()` loop, and `/api/switch` web endpoint |
| `_IOT_REAL_TIME` | off | Enables NTP synchronisation, timezone/DST conversion, and `printDateTime()` on displays |
| `WM_REMOTE_UPDATE` | off | Enables remote OTA via manifest URL; adds the remote-update UI to the Settings page |
| `WM_GEO_LOCATION` | off | Enables latitude/longitude/altitude settings page and `GeoSettings` NVS class |

### Identity flags

| Flag | Type | Default | Description |
|---|---|---|---|
| `SW_MAJOR_VERSION` | int | `1` | Firmware major version — appears in portal, HA device info, and manifest |
| `SW_MINOR_VERSION` | int | `0` | Firmware minor version |
| `SW_PATCH_VERSION` | int | `0` | Firmware patch version |
| `_IOT_DEVICE_NAME` | string | `"device"` | Human-readable device name shown in portal and HA |
| `_IOT_DEVICE_MODEL` | string | `""` | Model identifier |
| `_IOT_DEVICE_MANUFACTURER` | string | `""` | Manufacturer string |
| `_IOT_COPYRIGHT` | string | `""` | Copyright text in portal footer |

### Runtime / config flags

| Flag | Type | Default | Description |
|---|---|---|---|
| `_IOT_OTA_UPDATE_URL` | string | — | Full URL to `manifest.json` on your update server (required when `WM_REMOTE_UPDATE` is set) |
| `_IOT_DEBUG_LOGLEVEL_` | int | `0` | Log verbosity: 0 = off, 1 = error, 2 = warn, 3 = info, 4 = debug |

### Language flags

| Flag | Description |
|---|---|
| `LANGUAGE_EN_US` | English string set (used automatically if neither flag is defined) |
| `LANGUAGE_SK_SK` | Slovak string set |
