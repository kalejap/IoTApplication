# IoTApplication

Framework for building WiFi-connected IoT devices on **ESP8266** and **ESP32** with:

- WiFi configuration portal (via [ESPAsyncWiFiManager](https://github.com/kalejap/ESPAsyncWiFiManager))
- Home Assistant integration via MQTT (`ArduinoHA`)
- OTA firmware updates (local upload and remote manifest-based)
- NTP time synchronisation with timezone support
- Async web server with extensible HTTP endpoints
- LCD/character display with automatic page cycling
- NVS-backed settings persistence

---

## Dependencies

| Library | Purpose |
|---|---|
| `ESPAsyncWiFiManager` | WiFi config portal, web server, OTA |
| `ArduinoHA` | Home Assistant MQTT device/entity management |
| `ESPAsyncWebServer` | Async HTTP server |
| `ESPAsyncTCP` / `AsyncTCP` | Async TCP layer |
| `ESPAsyncDNSServer` | Captive portal DNS |
| `NTPClient` | NTP time sync (requires `_IOT_REAL_TIME`) |
| `Timezone` | DST-aware local time conversion |
| `TimeLib` | Arduino time functions |

---

## Architecture

```
main.cpp
  ├── MyDevice   : IoTDevice        — sensor/switch components, display pages
  └── MyApp      : IoTApplication   — WiFi, MQTT, OTA, web server, NTP

IoTDevice::preSetup()     — hardware init, HA device config
IoTDevice::postSetup()    — sensor/entity config (names, icons, callbacks)
IoTApplication::loop()
  ├── IoTDevice::preLoop()          — read sensors
  ├── updateAllComponents()         — poll hardware
  ├── publishAllComponents()        — push to MQTT
  └── IoTDevice::postLoop()         — tickDisplayPages()
```

---

## Quick start

```cpp
#include <IoTApplication.h>
#include <IoTDevice.h>

class MyDevice : public IoTDevice
{
public:
    MyDevice() : IoTDevice(IOT_DEVICE_PROPERTIES(dstRule, stdRule))
    {
        registerComponent(m_relay);
        registerPage(m_page);
        setDisplay(m_lcd);
    }

    void postSetup() override
    {
        m_relay.setName("Heater relay");
        m_relay.setIcon("mdi:radiator");
    }

private:
    IoTHASwitchWrapper            m_relay{D1, "relay_heat"};
    MyDisplayPage                 m_page;
    IoTLCDDisplay                 m_lcd{0x27, 16, 2};
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

---

## IoTDevice — virtual hooks

Override these in your device subclass:

| Method | When to override |
|---|---|
| `preSetup()` | Hardware initialisation (pin modes, bus init) |
| `postSetup()` | Configure HA entity names, icons, callbacks after WiFi is up |
| `isConfigTriggeredOnStartUp()` | Return `true` to force the WiFi portal on every boot |
| `onConfig(ssid)` | Called when the config portal opens |
| `onAddConfigRoutes(server)` | Register additional HTTP routes in the portal |
| `onCustomHeadElement()` | Return PROGMEM HTML to inject into portal `<head>` |
| `onBuildConfigHtml(html)` | Append custom form fields to the portal page |
| `onCustomIndexButtons()` | Return PROGMEM HTML for extra buttons on the index page |
| `onCustomSettingsButtons()` | Return PROGMEM HTML for extra buttons on the settings page |
| `onSaveConfigParameters()` | Read custom portal form values after the user saves |
| `preLoop()` | Read sensors before `updateAllComponents()` |
| `postLoop()` | Extra work after publish; base implementation calls `tickDisplayPages()` |
| `onSystemEvent(event)` | React to OTA / WiFi / MQTT / restart events (base freezes display) |
| `onUpdateDisplay(display)` | Manual display control (bypasses automatic page cycling) |

---

## Home Assistant components (`WM_SUPPORT_HOME_ASSISTANT`)

### Registering components

```cpp
// In IoTDevice constructor:
registerComponent(m_relay);
registerComponent(m_tempSensor);
```

`begin()` is called automatically during `preSetup()`.  
`update()` and `publishValue()` are called automatically every loop cycle.

### IoTHASwitchWrapper — GPIO on/off switch or relay

```cpp
IoTHASwitchWrapper(uint8_t pin, const char* uid, bool activeHigh = true)
```

| Method | Description |
|---|---|
| `setName(name)` | Display name in Home Assistant |
| `setIcon(icon)` | MaterialDesignIcons icon (e.g. `"mdi:lightbulb"`) |
| `setDeviceClass(cls)` | HA device class |
| `setRetain(bool)` | MQTT retain flag |
| `setOptimistic(bool)` | Update UI without waiting for state confirmation |
| `getCurrentState()` | Current switch state (`true` = ON) |
| `onCommand(callback)` | Callback fired after each state change |

Callback signature: `void(bool state, IoTHASwitchWrapper* sender)`

```cpp
m_relay.onCommand([](bool state, IoTHASwitchWrapper* sw) {
    Serial.println(state ? "ON" : "OFF");
});
```

### IoTHASensorNumberWrapper\<T\> — numeric sensor

```cpp
IoTHASensorNumberWrapper<float> m_temp{"sensor_temp",
    HABaseDeviceType::PrecisionP1};
```

| Method | Description |
|---|---|
| `setCurrentValue(T value)` | Set the current reading |
| `setName(name)` | Display name |
| `setDeviceClass(cls)` | HA device class (e.g. `"temperature"`) |
| `setStateClass(cls)` | State class for long-term statistics |
| `setUnitOfMeasurement(unit)` | Unit string (e.g. `"°C"`, `"%"`) |
| `setIcon(icon)` | MaterialDesignIcons icon |
| `setForceUpdate(bool)` | Send every value even if unchanged |

### IoTHACompositeDeviceWrapper\<Wrappers...\> — multi-entity component

Groups several HA entities from one physical component (e.g. BME280 → temperature + humidity + pressure):

```cpp
class BME280Wrapper : public IoTHACompositeDeviceWrapper<
    IoTHASensorNumberWrapper<float>,   // 0: temperature
    IoTHASensorNumberWrapper<float>,   // 1: humidity
    IoTHASensorNumberWrapper<float>>   // 2: pressure
{
public:
    BME280Wrapper()
        : IoTHACompositeDeviceWrapper(
            IoTHASensorNumberWrapper<float>{"temp",  PrecisionP1},
            IoTHASensorNumberWrapper<float>{"humid", PrecisionP0},
            IoTHASensorNumberWrapper<float>{"press", PrecisionP0}) {}

    bool update(bool force) override
    {
        get<0>().setCurrentValue(bme.readTemperature());
        get<1>().setCurrentValue(bme.readHumidity());
        get<2>().setCurrentValue(bme.readPressure() / 100.0f);
        return true;
    }
};
```

---

## Display system

### IoTTextDisplay — abstract display interface

Implement for any character display. Pure virtual methods:

| Method | Description |
|---|---|
| `begin()` | Initialise hardware |
| `clear()` | Clear entire display |
| `cols() / rows()` | Display dimensions |
| `setCursor(col, row)` | Move cursor (0-based) |
| `print(text / value)` | Print string, int, or float |
| `printLine(row, text)` | Print at column 0 and pad with spaces (no flicker) |

Optional virtual no-ops:

| Method | Description |
|---|---|
| `setBacklight(bool)` | Backlight on/off |
| `createChar(index, bitmap)` | Define custom character in slot 0–7 |
| `writeChar(index)` | Print custom character |
| `onSystemEvent(event)` | Show OTA/restart status text |
| `printDateTime()` | Print time (row 0) and date (row 1) — `_IOT_REAL_TIME` only |

### IoTDisplayPage — abstract display page

```cpp
class MyPage : public IoTDisplayPage
{
public:
    void render(IoTTextDisplay& display) override
    {
        display.printLine(0, "Hello");
        display.printLine(1, "World");
    }

    unsigned long durationMs() const override { return 3000; }  // 3 s
};
```

Register with: `registerPage(myPage);`

Pages cycle automatically. Call `freezeDisplay()` / `unfreezeDisplay()` to pause cycling (done automatically during OTA and restart events).

---

## System events

`IoTDevice::onSystemEvent()` is called for all lifecycle events. The base implementation freezes the display on `OTA_START` / `RESTARTING` and delegates to `display.onSystemEvent()`.

```cpp
void onSystemEvent(const IoTSystemEvent& event) override
{
    IoTDevice::onSystemEvent(event);  // call base first
    if (event.type == IoTSystemEvent::Type::WIFI_CONNECTED)
    {
        IPAddress ip(event.arg1);
        // ...
    }
}
```

| Event type | Extra fields |
|---|---|
| `OTA_START` | — |
| `OTA_PROGRESS` | `arg1` = bytes uploaded, `arg2` = total bytes |
| `OTA_END` | `flag` = `true` if success |
| `WIFI_CONNECTED` | `arg1` = IP address as `uint32_t` |
| `WIFI_DISCONNECTED` | — |
| `MQTT_CONNECTED` | — |
| `MQTT_DISCONNECTED` | — |
| `RESTARTING` | — |

---

## Settings persistence

All settings use NVS (Non-Volatile Storage) via the `Settings` base class.

| Class | NVS namespace | What it stores |
|---|---|---|
| `AppSettings` | `"APP"` | Temperature unit (Celsius / Fahrenheit) |
| `WiFiSettings` | `"WIFI"` | SSID, password, static IP, gateway, subnet |
| `MQTTSettings` | `"MQTT"` | Broker host, port, username, password |

```cpp
WiFiSettings wifi;
wifi.read();
wifi.setSSID("MyNetwork");
wifi.save();          // saves only if isDirty()
```

Custom settings: subclass `Settings`, implement `readFields()` and `saveFields()`,
use the `updateValue()` helper to auto-track the dirty flag.

---

## Compile-time flags

| Flag | Effect |
|---|---|
| `WM_SUPPORT_HOME_ASSISTANT` | Enables MQTT, ArduinoHA entities, component registry, `/api/switch` web endpoint |
| `_IOT_REAL_TIME` | Enables NTP sync, timezone conversion, `printDateTime()` |
| `WM_REMOTE_UPDATE` | Enables remote OTA via JSON manifest URL |
| `LANGUAGE_EN_US` / `LANGUAGE_SK_SK` | Selects localised string set |
| `_IOT_DEBUG_LOGLEVEL_` | Log verbosity: 0=off 1=error 2=warn 3=info 4=debug |

---

## License

LGPL 2.1 — see [LICENSE](LICENSE) file.
