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

#include <ArduinoHA.h> // Home Assistant
#include <Timezone.h> // Time zone

/**
 * Structure to represent semantic version in form X.Y.Z
 * A - Major version
 * Y - Minor version
 * X - Patch version
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
    PGM_P deviceManufacturer;

    /**
     * @brief software version as X.Y.Z
     */
    SemanticVersion version;

    /**
     * @brief String version of software version as "X.Y.Z"
     */
    PGM_P versionString;

    /**
     * @brief URL for OTA software update.
     */
    PGM_P OTAupdateUrl;

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
    IoTDevice(const DeviceProperties& properties) :
        _properties(properties)
    {}
    virtual ~IoTDevice()
    {}
    IoTDevice(const IoTDevice&) = delete;
    IoTDevice& operator=(const IoTDevice&) = delete;
    IoTDevice(IoTDevice&& src) noexcept = delete;
    IoTDevice& operator=(IoTDevice&& rhs) noexcept = delete;


    /**
     * @brief pre-setup method called on setup of IoT application
     */
    virtual void preSetup()
    {};

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
     * @brief pre-loop method called from IoT application's loop
     */
    virtual void preLoop()
    {};

    /**
     * @brief post-loop method called from IoT application's loop
     */
    virtual void postLoop()
    {};

    /**
     * @brief Get access to Home assistant MQTT device
     */
    virtual HADevice& device() = 0;

    /**
     * @brief Get access to device properties
     */
    const DeviceProperties& deviceProperties()
    {
        return _properties;
    }

public:
    const DeviceProperties& _properties;
};

#endif // IOTDEVICE_H
