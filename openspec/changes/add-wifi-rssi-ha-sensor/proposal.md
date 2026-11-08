## Why

Every WiFi-connected IoT device benefits from monitoring signal strength in Home Assistant — RSSI history reveals intermittent connectivity problems and helps with physical placement decisions. Currently there is no built-in HA sensor for this; each project would have to wire one up manually.

## What Changes

- Add `ESP8266WifiRssiSensor` class to the IoTApplication library — a thin subclass of `IoTHASensorNumberWrapper<int>` that reads `WiFi.RSSI()` on each update cycle.
- Add `_wifiRssiSensor` as a built-in member of `IoTDevice`, auto-registered in the constructor (Option B — zero boilerplate in device subclasses).
- Publish only when WiFi is connected; explicitly mark the HA entity **unavailable** when the link is down.
- Include RSSI in the `statusJSON()` hardware-status web panel alongside reboot counters.
- Add `L_WIFI_RSSI_SENSOR_NAME` to the IoTApplication library language files (`iot_en_US.h`, `iot_sk_SK.h`), with `"WiFi RSSI"` in both languages. Add a language selector header `IoTLanguageSupport.h`.

## Capabilities

### New Capabilities

- `wifi-rssi-sensor`: WiFi RSSI is published as a Home Assistant numeric sensor with `device_class: signal_strength`, unit `dBm`, `state_class: measurement`, and per-entity availability tracking. The sensor is a built-in component of `IoTDevice` — no registration code is required in device subclasses.

### Modified Capabilities

## Impact

- `IoTApplication` library: new files `ESP8266WifiRssiSensor.h`, `IoTLanguageSupport.h`, `language/iot_en_US.h`, `language/iot_sk_SK.h`; modified `IoTDevice.h` (new member + auto-register call).
- All existing `IoTDevice` subclasses gain the RSSI entity automatically — no code changes needed in applications.
- ArduinoHA dependency used as-is (no version change required).
