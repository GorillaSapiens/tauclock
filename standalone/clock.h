//  Sunclock, draw a clock with local solar and lunar information
//  Copyright (C) 2022,2023 Adam Wozniak / GorillaSapiens
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _INCLUDE_CLOCK_H_
#define _INCLUDE_CLOCK_H_

#include "draw.h"

Canvas *do_all(double lat,
               double lon,
               double offset,
               int width,
               const char *locprovider,
               const char *tzprovider,
               const char *tz,
               int lightdark,
               const char *monam[],
               const char *wenam[]);

int do_when_is_it(double lat, double lon, int category, int type, int delayMinutes);

#endif
