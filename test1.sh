#!/bin/sh

##  Sunclock, draw a clock with local solar and lunar information
##  Copyright (C) 2022 Adam Wozniak / GorillaSapiens
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <https://www.gnu.org/licenses/>.

./calcdata 34.0007 -81.0348 -4 | ./clock.pl | grep disk
mv out.png full.png

./calcdata 34.0007 -81.0348 0 | ./clock.pl | grep disk
mv out.png full_left.png

./calcdata 34.0007 -81.0348 4 | ./clock.pl | grep disk
mv out.png left.png

./calcdata 34.0007 -81.0348 7 | ./clock.pl | grep disk
mv out.png left_new.png

./calcdata 34.0007 -81.0348 10 | ./clock.pl | grep disk
mv out.png new.png

./calcdata 34.0007 -81.0348 14 | ./clock.pl | grep disk
mv out.png new_right.png

./calcdata 34.0007 -81.0348 18 | ./clock.pl | grep disk
mv out.png right.png

./calcdata 34.0007 -81.0348 22 | ./clock.pl | grep disk
mv out.png right_full.png

