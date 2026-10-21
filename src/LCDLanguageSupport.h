/*
  LCDLanguageSupport.h - Selects the correct language header for LCD display strings.
  Copyright (c) 2026 Peter Kaleja.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  Define LANGUAGE_SK_SK in build flags to select Slovak; English is the default.
*/

#pragma once
#if defined(LANGUAGE_SK_SK)
    #include "language/lcd_sk_SK.h"
#else
    #include "language/lcd_en_US.h"
#endif
