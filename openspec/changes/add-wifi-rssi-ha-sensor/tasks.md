## 1. Language files

- [x] 1.1 Create `src/language/iot_en_US.h` — define `L_WIFI_RSSI_SENSOR_NAME "WiFi RSSI"`
- [x] 1.2 Create `src/language/iot_sk_SK.h` — define `L_WIFI_RSSI_SENSOR_NAME "WiFi RSSI"`
- [x] 1.3 Create `src/IoTLanguageSupport.h` — selector header: include `iot_sk_SK.h` when `LANGUAGE_SK_SK` is defined, otherwise `iot_en_US.h`

## 2. ESP8266WifiRssiSensor class

- [x] 2.1 Create `src/ESP8266WifiRssiSensor.h` guarded by `#ifdef WM_SUPPORT_HOME_ASSISTANT`
- [x] 2.2 Class extends `IoTHASensorNumberWrapper<int>`, constructor passes uid `"wifi_rssi"` to base
- [x] 2.3 `begin()` — calls `setAvailability(true)` on `_sensor`, sets `device_class "signal_strength"`, unit `"dBm"`, state_class `"measurement"`, icon `"mdi:wifi"`, and name from `L_WIFI_RSSI_SENSOR_NAME`
- [x] 2.4 `update()` — if `WiFi.status() != WL_CONNECTED`: call `_sensor.publishAvailability(false)` and return; else read `WiFi.RSSI()`, skip if zero, call `setCurrentValue(rssi)` and `_sensor.publishAvailability(true)`
- [x] 2.5 `statusJSON()` — return a `{"name":"WiFi RSSI","value":<rssi>}` JSON object using `JSONUtils`

## 3. IoTDevice integration

- [x] 3.1 In `IoTDevice.h`, include `ESP8266WifiRssiSensor.h` under `#ifdef WM_SUPPORT_HOME_ASSISTANT`
- [x] 3.2 Add `ESP8266WifiRssiSensor _wifiRssiSensor` as a private member under `#ifdef WM_SUPPORT_HOME_ASSISTANT`
- [x] 3.3 In `IoTDevice` constructor body, call `registerComponent(_wifiRssiSensor)` under `#ifdef WM_SUPPORT_HOME_ASSISTANT`

## 4. Verify

- [x] 4.1 Build `SolidFuelBoilerController` (debug env) — zero errors and warnings
- [ ] 4.2 Flash device and confirm `wifi_rssi` entity appears in Home Assistant under the device
- [ ] 4.3 Disconnect WiFi (or simulate) and confirm entity shows as unavailable in HA
- [ ] 4.4 Confirm RSSI value appears in the hardware-status web panel (`/sq?dx=hwstatus`)
