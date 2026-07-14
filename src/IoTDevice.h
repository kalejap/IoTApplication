/*
  IIoTDevice.h - Abstract class that represents an IoT device.
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

#ifndef IOTDEVICE_H
#define IOTDEVICE_H

#ifdef WM_SUPPORT_HOME_ASSISTANT
    #include <ArduinoHA.h> // Home Assistant
    #include "IoTHADeviceWrapperBase.h"
#endif

#include <Timezone.h> // Time zone
#include "DeviceDefines.h" // Software version
#include "IoTTextDisplay.h"
#include "IoTDisplayPage.h"
#include "IoTSystemEvent.h"
#include "Timer.h"

// Forward declaration — avoids pulling AsyncWebServer into every TU that includes IoTDevice.h
class AsyncWebServer;


#define IOT_DEVICE_PROPERTIES(SUMMER_TIME_SR, STANDARD_TIME_SR) \
    const char chipName[] PROGMEM = IOT_CHIP_NAME; \
    const char deviceName[] PROGMEM = IOT_DEVICE_NAME; \
    const char deviceModel[] PROGMEM = IOT_DEVICE_MODEL; \
    const char deviceManufacturer[] PROGMEM = IOT_DEVICE_MANUFACTURER; \
    const char hardwareId[] PROGMEM = IOT_MAKE_HARDWARE_ID; \
    const char versionString[] PROGMEM = IOT_SW_VERSION_STRING; \
    const char hardwareIdTag[] PROGMEM =  IOT_HARDWARE_TAG_PREFIX IOT_MAKE_HARDWARE_ID; \
    const char versionStringTag[] PROGMEM = IOT_VERSION_TAG_PREFIX IOT_SW_VERSION_STRING; \
    const char languageTag[] PROGMEM = IOT_LANGUAGE_TAG_PREFIX IOT_LANGUAGE_STRING; \
    const char OTAupdateUrl[] PROGMEM = IOT_OTA_UPDATE_URL; \
    DeviceProperties devProperties = { \
        chipName, \
        deviceName, \
        deviceModel, \
        deviceManufacturer, \
        hardwareId, \
        versionString, \
        hardwareIdTag, \
        versionStringTag, \
        languageTag, \
        OTAupdateUrl, \
        { SW_MAJOR_VERSION, SW_MINOR_VERSION, SW_PATCH_VERSION }, \
        SUMMER_TIME_SR, \
        STANDARD_TIME_SR, \
    }


/**
 * Structure to represent semantic version in form X.Y.Z
 * X - Major version
 * Y - Minor version
 * Z - Patch version
 */
struct SemanticVersion
{
    bool operator>(const SemanticVersion& rhs) const
    {
        if (_majorVersion < rhs._majorVersion)
        {
            return false;
        }
        else if (_majorVersion > rhs._majorVersion)
        {
            return true;
        }

        if (_minorVersion < rhs._minorVersion)
        {
            return false;
        }
        else if (_minorVersion > rhs._minorVersion)
        {
            return true;
        }

        if (_patchVersion > rhs._patchVersion)
        {
            return true;
        }

        return false;
    }

    bool operator= (const SemanticVersion& rhs) const
    {
        return (_majorVersion == rhs._majorVersion &&
                _minorVersion == rhs._minorVersion &&
                _patchVersion == rhs._patchVersion);
    }

    static SemanticVersion fromString(const String& s) {
        SemanticVersion v = {0, 0, 0};
        sscanf(s.c_str(), "%hu.%hu.%hu",
               &v._majorVersion, &v._minorVersion, &v._patchVersion);
        return v;
    }

    unsigned short _majorVersion;
    unsigned short _minorVersion;
    unsigned short _patchVersion;
};

/**
 * @brief Device properties
 */
struct DeviceProperties
{
    /**
     * @brief Unique ID of the device (e.g. MAC address)
     */
    PGM_P chipName;

    /**
     * @brief device name that appears in Home Assistant
     */
    PGM_P deviceName;

    /**
     * @brief device model that appears in Home Assistant
     */
    PGM_P deviceModel;

    /**
     * @brief device manufacturer that appears in Home Assistant
     */
    PGM_P manufacturer;

    /**
     * @brief Hardware ID of the device (e.g. "ESP8266|Thermostat|A1|PK DIY")
     */
    PGM_P hardwareId;

    /**
     * @brief String version of software version as "X.Y.Z"
     */
    PGM_P versionString;

    /**
     * @brief Hardware ID tag for OTA update check
     * e.g. "<MAGIGSTRING>:hw:ESP8266|Thermostat|A1|PK DIY"
     */
    PGM_P hardwareIdTag;

    /**
     * @brief Version tag for OTA update check
     * e.g. "<MAGIGSTRING>:ve:1.0.0"
     */
    PGM_P versionStringTag;

    /**
     * @brief Language tag embedded in the binary for OTA scanning.
     * e.g. "@*MAGic*@:lg:en-us"
     */
    PGM_P languageTag;

    /**
     * @brief URL for OTA software update.
     */
    PGM_P OTAupdateUrl;

    /**
     * @brief software version as X.Y.Z
     */
    SemanticVersion version;

    /**
     * @brief rule for start of dst or summer time for any year
     */
    TimeChangeRule dstStart;

    /**
     * @brief rule for start of standard time for any year
     */
    TimeChangeRule stdStart;
};


/**
 * @brief Abstract class to represent an IoT device.
 */
class IoTDevice
{
  public:
    IoTDevice(const DeviceProperties& properties);
    virtual ~IoTDevice();
    IoTDevice(const IoTDevice&) = delete;
    IoTDevice& operator=(const IoTDevice&) = delete;
    IoTDevice(IoTDevice&& src) noexcept = delete;
    IoTDevice& operator=(IoTDevice&& rhs) noexcept = delete;


    /**
     * @brief pre-setup method called on setup of IoT application
     */
    virtual void preSetup();

    /**
     * @brief pre-setup method called on setup of IoT application
     */
    virtual void postSetup()
    {};

    /**
     * @brief Method to check if configuration settings is triggered
     * on start up of IoT application
     */
    virtual bool isConfigTriggeredOnStartUp ()
    { return false; };

    /**
     * @brief Method called on start of configution
     * ssid - name of SSID
     */
    virtual void onConfig(const String& ssid)
    {}

    /**
     * @brief Called from configure() so the device can register extra HTTP routes
     *        on the async web server (e.g. a live JSON data endpoint or static asset).
     *        Invoked before startConfigPortal().
     * @param server  Reference to the application's AsyncWebServer instance.
     */
    virtual void onAddConfigRoutes(AsyncWebServer& server)
    {}

    /**
     * @brief Return a pointer to a static const char[] to be injected into the portal
     *        <head> via setCustomHeadElement(). Return nullptr for no custom head element.
     *        The pointer must remain valid for the lifetime of the portal.
     */
    virtual const char* onCustomHeadElement()
    { return nullptr; }

    /**
     * @brief Append custom HTML to html (table, buttons, inline <script>, etc.).
     *        Called by configure() before startConfigPortal(); IoTApplication wraps
     *        the result in an ESPAsync_WMParameter and adds it to the portal form.
     *        Do nothing (default) if no custom HTML is needed.
     */
    virtual void onBuildConfigHtml(String& html)
    {}

    /**
     * @brief Return PROGMEM pointer to one or more <a class='mainbtn'> elements injected
     *        after the Settings button on the portal index page.
     *        Return nullptr (default) for no extra buttons.
     *        Pointer must be stable for the application lifetime.
     */
    virtual PGM_P onCustomIndexButtons()
    { return nullptr; }

    /**
     * @brief Return PROGMEM pointer to one or more <a class='mainbtn'> elements injected
     *        between the OTA and Restart buttons on the /settings page.
     *        Return nullptr (default) for no extra buttons.
     *        Pointer must be stable for the application lifetime.
     */
    virtual PGM_P onCustomSettingsButtons()
    { return nullptr; }

    /**
     * @brief Called from configure() after the user saves the portal form.
     *        Read any WMParameter values or perform other post-save cleanup here.
     */
    virtual void onSaveConfigParameters()
    {}

    /**
     * @brief pre-loop method called from IoT application's loop
     */
    virtual void preLoop()
    {};

    /**
     * @brief post-loop method called from IoT application's loop
     */
    virtual void postLoop()
    {
        tickDisplayPages();
    }

#ifdef WM_SUPPORT_HOME_ASSISTANT
    /**
     * @brief Get access to Home assistant MQTT device
     */
    HADevice& device()
    {
        return _device;
    }

    /**
     * @brief Call update(force) on every registered component in registration order.
     *        Drives hardware polling (e.g. temperature conversion) before publishing.
     */
    void updateAllComponents(bool force = false);

    /**
     * @brief Call publishValue(force) on every registered component.
     */
    void publishAllComponents(bool force = false);

    /**
     * @brief Assemble statusJSON() results from all components into a flat JSON array.
     *        Returns "[]" when no component has status to report.
     */
    String allComponentsStatusJSON() const;

    /**
     * @brief Dispatch a web UI command to the first component that claims uid.
     * @return true if a component handled the command, false if uid was not found.
     */
    bool dispatchWebCommand(const char* uid, bool state);
#endif

    /**
     * @brief Returns true if a display has been registered with setDisplay().
     */
    bool hasDisplay() const { return _pDisplay != nullptr; }

    /**
     * @brief Called by IoTApplication when a system-level event occurs
     *        (OTA lifecycle, WiFi connect/disconnect, MQTT connect/disconnect).
     *        Default implementation freezes/unfreezes the display page cycle and
     *        delegates text rendering to the registered display.
     *        Override and call IoTDevice::onSystemEvent(event) to add extra behaviour.
     */
    virtual void onSystemEvent(const IoTSystemEvent& event)
    {
        using T = IoTSystemEvent::Type;
        switch (event.type)
        {
            case T::OTA_START:
            case T::RESTARTING:
                freezeDisplay();
                break;

            case T::OTA_END:
                if (!event.flag)
                {
                    unfreezeDisplay();
                }
                break;

            default:
                break;
        }
        if (_pDisplay)
        {
            _pDisplay->onSystemEvent(event);
        }
    }

    /**
     * @brief Get access to device properties
     */
    const DeviceProperties& deviceProperties()
    {
        return _properties;
    }

    /**
     * @brief Advance the display page cycle by one tick.
     *        Does nothing when no display is registered, when the display is frozen
     *        (see freezeDisplay()), or when no pages have been registered.
     *        Called automatically via postLoop(); may also be called directly to
     *        force an immediate display refresh.
     */
    void tickDisplayPages();

protected:
    /**
     * @brief Suspend page cycling so direct display writes (e.g. from
     *        onSystemEvent) are not overwritten by the 2-second tick.
     *        Call freezeDisplay() at the start of an OTA/event and
     *        unfreezeDisplay() when the device is ready to resume normal pages.
     */
    void freezeDisplay()   { _displayFrozen = true; }
    void unfreezeDisplay() { _displayFrozen = false; }

    /**
     * @brief Register a display. Typically called from the derived class constructor.
     */
    void setDisplay(IoTTextDisplay& display) { _pDisplay = &display; }

    /**
     * @brief Register a display page. Pages are shown in registration order.
     *        Call from the derived class constructor or preSetup().
     */
    void registerPage(IoTDisplayPage& page);

    /**
     * @brief Override to control display content manually. The default implementation
     *        cycles through registered pages, respecting each page's durationMs().
     */
    virtual void onUpdateDisplay(IoTTextDisplay& display);

#ifdef WM_SUPPORT_HOME_ASSISTANT
    /**
     * @brief Register a HW component so the base class can iterate it.
     *        Call from the derived class constructor for each member component.
     */
    void registerComponent(IoTHADeviceWrapperBase& component);

    /**
     * @brief Call a method on every registered component, forwarding the given arguments.
     *
     * @tparam Ret    Return type of the method (discarded; use publishAllComponents /
     *                updateAllComponents if you need return values).
     * @tparam Args   Method parameter types (deduced automatically).
     * @param  method Pointer-to-member of IoTHADeviceWrapperBase, e.g.
     *                &IoTHADeviceWrapperBase::update
     * @param  args   Arguments forwarded to every call, e.g. force flag.
     *
     * Example:
     *   forEachComponent(&IoTHADeviceWrapperBase::update, true);
     *   forEachComponent(&IoTHADeviceWrapperBase::publishValue, false);
     */
    template<typename Ret, typename... Args>
    void forEachComponent(Ret (IoTHADeviceWrapperBase::*method)(Args...), Args... args)
    {
        for (uint8_t i = 0; i < _componentCount; ++i)
            (_components[i]->*method)(args...);
    }
#endif

public:
    const DeviceProperties& _properties;

#ifdef WM_SUPPORT_HOME_ASSISTANT
    /**
     * @brief Home assistant device
     */
    HADevice _device;

private:
    static constexpr uint8_t MAX_COMPONENTS = IOT_MAX_COMPONENTS;
    IoTHADeviceWrapperBase* _components[MAX_COMPONENTS] = {};
    uint8_t _componentCount = 0;
#endif

private:
    // Display / page cycling
    static constexpr uint8_t MAX_DISPLAY_PAGES = 6;
    IoTTextDisplay*  _pDisplay            = nullptr;
    IoTDisplayPage*  _displayPages[MAX_DISPLAY_PAGES] = {};
    uint8_t          _displayPageCount    = 0;
    uint8_t          _currentPageIndex    = 0;
    Timer            _displayPageTimer{5000};
    bool             _displayTimerReady   = false;
    bool             _displayFrozen       = false;


};

#endif // IOTDEVICE_H
