/*
  IoTDisplayPage.h - Abstract interface for a single display page.
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

#include "IoTTextDisplay.h"

/**
 * @brief Abstract base for a single display page.
 *
 * Derive a concrete page class for each screen of content you want to show.
 * Register pages with IoTDevice::registerPage(). The framework cycles through
 * registered pages automatically, calling render() on the active page at each
 * display refresh tick.
 */
class IoTDisplayPage
{
public:
    virtual ~IoTDisplayPage() = default;

    /**
     * @brief Draw this page's content onto the display.
     *        Called by the framework on every display tick while this page
     *        is active. Use display.printLine() to overwrite content in-place
     *        without a full clear() to avoid flicker.
     */
    virtual void render(IoTTextDisplay& display) = 0;

    /**
     * @brief How long (ms) this page stays visible before the framework
     *        advances to the next page. Default: 5 seconds.
     */
    virtual unsigned long durationMs() const { return 5000UL; }
};
