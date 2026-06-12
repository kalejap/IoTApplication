/*
  IoTDebug.h - Macros to print errors, warnings and debug info

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

#pragma once

#ifndef IOTDEBUG_H
#define IOTDEBUG_H

#define IOT_DEBUG_SERIAL      Serial

// Change _IOT_DEBUG_LOGLEVEL_ to set tracing and logging verbosity
// 0: DISABLED: no logging
// 1: ERROR: errors
// 2: WARN: errors and warnings
// 3: INFO: errors, warnings and informational (default)
// 4: DEBUG: errors, warnings, informational and debug

#ifndef _IOT_DEBUG_LOGLEVEL_
  #define _IOT_DEBUG_LOGLEVEL_       1
#endif

/////////////////////////////////////////////////////////

const char IOT_DEBUG_MARK[] = "[IOT] ";
const char IOT_DEBUG_SP[]   = " ";

#define IOT_DEBUG_PRINT        IOT_DEBUG_SERIAL.print
#define IOT_DEBUG_PRINTLN      IOT_DEBUG_SERIAL.println

#define IOT_DEBUG_PRINT_MARK   IOT_DEBUG_PRINT(IOT_DEBUG_MARK)
#define IOT_DEBUG_PRINT_SP     IOT_DEBUG_PRINT(IOT_DEBUG_SP)

/////////////////////////////////////////////////////////

#define IOTLOGERROR(x)         if(_IOT_DEBUG_LOGLEVEL_>0) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINTLN(x); }
#define IOTLOGERROR0(x)        if(_IOT_DEBUG_LOGLEVEL_>0) { IOT_DEBUG_PRINT(x); }
#define IOTLOGERROR1(x,y)      if(_IOT_DEBUG_LOGLEVEL_>0) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(y); }
#define IOTLOGERROR2(x,y,z)    if(_IOT_DEBUG_LOGLEVEL_>0) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(y); IOT_DEBUG_PRINT_SP;  IOT_DEBUG_PRINTLN(z); }
#define IOTLOGERROR3(x,y,z,w)  if(_IOT_DEBUG_LOGLEVEL_>0) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(y); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(z); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(w); }

/////////////////////////////////////////////////////////

#define IOTLOGWARN(x)          if(_IOT_DEBUG_LOGLEVEL_>1) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINTLN(x); }
#define IOTLOGWARN0(x)         if(_IOT_DEBUG_LOGLEVEL_>1) { IOT_DEBUG_PRINT(x); }
#define IOTLOGWARN1(x,y)       if(_IOT_DEBUG_LOGLEVEL_>1) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(y); }
#define IOTLOGWARN2(x,y,z)     if(_IOT_DEBUG_LOGLEVEL_>1) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(y); IOT_DEBUG_PRINT_SP;  IOT_DEBUG_PRINTLN(z); }
#define IOTLOGWARN3(x,y,z,w)   if(_IOT_DEBUG_LOGLEVEL_>1) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(y); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(z); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(w); }

/////////////////////////////////////////////////////////

#define IOTLOGINFO(x)          if(_IOT_DEBUG_LOGLEVEL_>2) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINTLN(x); }
#define IOTLOGINFO0(x)         if(_IOT_DEBUG_LOGLEVEL_>2) { IOT_DEBUG_PRINT(x); }
#define IOTLOGINFO1(x,y)       if(_IOT_DEBUG_LOGLEVEL_>2) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(y); }
#define IOTLOGINFO2(x,y,z)     if(_IOT_DEBUG_LOGLEVEL_>2) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(y); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(z); }
#define IOTLOGINFO3(x,y,z,w)   if(_IOT_DEBUG_LOGLEVEL_>2) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(y); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(z); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(w); }

/////////////////////////////////////////////////////////

#define IOTLOGDEBUG(x)         if(_IOT_DEBUG_LOGLEVEL_>3) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINTLN(x); }
#define IOTLOGDEBUG0(x)        if(_IOT_DEBUG_LOGLEVEL_>3) { IOT_DEBUG_PRINT(x); }
#define IOTLOGDEBUG1(x,y)      if(_IOT_DEBUG_LOGLEVEL_>3) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(y); }
#define IOTLOGDEBUG2(x,y,z)    if(_IOT_DEBUG_LOGLEVEL_>3) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(y); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(z); }
#define IOTLOGDEBUG3(x,y,z,w)  if(_IOT_DEBUG_LOGLEVEL_>3) { IOT_DEBUG_PRINT_MARK; IOT_DEBUG_PRINT(x); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(y); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINT(z); IOT_DEBUG_PRINT_SP; IOT_DEBUG_PRINTLN(w); }

/////////////////////////////////////////////////////////

#endif // IOTDEBUG_H

