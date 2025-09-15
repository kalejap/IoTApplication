/*
  Settings.h - Base abstract class to manage settings persistently stored
  as prefereneces in Non-volatile space (NVS) of ESP32/ESP8266

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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Preferences.h>

/**
 * Base abstract classes to manage settings persistently stored
 * as prefereneces in Non-volatile space (NVS) of ESP32
*/
class Settings
{
public:
    Settings(PGM_P psName);
    Settings(const Settings&);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    Settings(Settings &&rval);
#endif

    // creates a copy of the assigned value.  if the value is null or
    // invalid, or if the memory allocation fails, the string will be
    // marked as invalid ("if (s)" will be false).
    Settings & operator =(const Settings &rhs);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    Settings & operator =(Settings &&rval);
#endif


    /**
     * @brief Read settings from on-board non-volatile memory (NVS) of ESP32
     */
    bool read();

    /**
     * @brief Save settings to on-board non-volatile memory (NVS) of ESP32
     */
    bool save() const;

    /**
     * @brief Check if settings has been modified and need to be saved
    */
    bool isDirty() const
    {
        return _isDirty;
    }

protected:
    /**
     * @brief Update setting's member
     */
    template <typename T>
    void updateValue(T value, T& member)
    {
        if (member != value)
        {
            member = value;
            _isDirty = true;
        }
    }

    /**
     * @brief Update setting's member
     */
    void updateValue(String value, String& member, bool bTrim)
    {
        if (bTrim)
        {
            value.trim();
        }

        if (member != value)
        {
            member = value;
            _isDirty = true;
        }
    }

protected:
    /**
     * @brief Read all fields from preferences (NVS of ESP32)
    */
    virtual void readFields(Preferences& pref) = 0;

    /**
     * @brief Save all fields to preferences (NVS of ESP32)
    */
    virtual bool saveFields(Preferences& pref) const = 0;

    /**
     * @brief Return namespace
    */
    PGM_P name() const;

protected:
    PGM_P _name;
    mutable bool _isDirty = false;
};

#endif // SETTINGS_H
