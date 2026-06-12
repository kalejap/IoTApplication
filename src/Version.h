/*
  Version.h - Defines current software version
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

#ifndef VERSION_H
#define VERSION_H

#ifndef SW_MAJOR_VERSION
    #define SW_MAJOR_VERSION 1
#endif

#ifndef SW_MINOR_VERSION
    #define SW_MINOR_VERSION 0
#endif

#ifndef SW_PATCH_VERSION
    #define SW_PATCH_VERSION 0
#endif

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define SW_VERSION_STRING STRINGIZE(SW_MAJOR_VERSION) "." STRINGIZE(SW_MINOR_VERSION) "." STRINGIZE(SW_PATCH_VERSION)

#endif // VERSION_H
