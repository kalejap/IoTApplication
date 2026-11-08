## Purpose

Exposes the ESP8266 WiFi received signal strength (RSSI in dBm) as a Home Assistant numeric sensor so that signal quality can be monitored, graphed, and alerted on from any HA dashboard.

## ADDED Requirements

### Requirement: RSSI published as HA numeric sensor

The system SHALL expose WiFi RSSI as a Home Assistant sensor entity with `device_class: signal_strength`, unit of measurement `dBm`, and `state_class: measurement`.

#### Scenario: RSSI reported while connected

- **WHEN** WiFi is connected and the update cycle fires
- **THEN** the sensor SHALL publish the current RSSI value (a negative integer in dBm) to Home Assistant

#### Scenario: Sensor marked unavailable when disconnected

- **WHEN** WiFi is not connected during an update cycle
- **THEN** the sensor SHALL publish the HA unavailability state instead of a numeric value, so Home Assistant shows the entity as unavailable rather than stale

#### Scenario: Zero RSSI skipped

- **WHEN** `WiFi.RSSI()` returns `0` (radio not yet associated)
- **THEN** the sensor SHALL NOT update the published value; the previous valid reading SHALL be retained

### Requirement: RSSI is a built-in IoTDevice component

The RSSI sensor SHALL be automatically available on every `IoTDevice` subclass without any registration code in device constructors or `postSetup()`.

#### Scenario: No registration required

- **WHEN** an application creates an `IoTDevice` subclass without calling `registerComponent()` for RSSI
- **THEN** the RSSI entity SHALL still appear in Home Assistant after the first MQTT connection

### Requirement: RSSI visible in hardware-status web panel

The RSSI sensor SHALL contribute its current value to the `/sq?dx=hwstatus` JSON endpoint so the web UI hardware-status panel displays it.

#### Scenario: RSSI in hwstatus response

- **WHEN** a client requests `/sq?dx=hwstatus`
- **THEN** the JSON response SHALL include a `name`/`value` object for WiFi RSSI

### Requirement: RSSI sensor name localizable

The sensor's display name in Home Assistant SHALL be controlled by the `L_WIFI_RSSI_SENSOR_NAME` macro defined in the IoTApplication language files.

#### Scenario: Default name in English and Slovak

- **WHEN** the firmware is built with either `LANGUAGE_SK_SK` or the English default
- **THEN** the sensor name in Home Assistant SHALL be `"WiFi RSSI"` in both cases
