/*
  Settings.cpp - Base abstract class to manage settings persistently stored
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

#include "Settings.h"

Settings::Settings(PGM_P psName) :
    _name(psName)
{}

Settings::Settings(const Settings&) = default;

Settings& Settings::operator =(const Settings &rhs) = default;


#ifdef __GXX_EXPERIMENTAL_CXX0X__
    Settings::Settings(Settings &&rval) = default;
    Settings& Settings::operator =(Settings &&rval) = default;
#endif

bool Settings::read()
{
    Preferences preferences;
    bool bNotSavedYes = false;
    if (!preferences.begin(name(), true))
    {
        // Initialize namespace
        if (!preferences.begin(name(), false))
        {
            // Something is wrong
            return false;
        }
    }

    readFields(preferences);
    if (bNotSavedYes)
    {
        saveFields(preferences);
    }
 
    preferences.end();

    return true;
}

bool Settings::save() const
{
    Preferences preferences;
    if (!preferences.begin(name(), false))
    {
        return false;
    }

    bool res = false;
    if (saveFields(preferences))
    {
        _isDirty = false;
        res = true;
    }

    preferences.end();

    return res;
}

PGM_P Settings::name() const
{
    return _name;
}