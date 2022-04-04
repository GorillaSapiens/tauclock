#!/usr/bin/perl

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

`rm out*.png *.mp4 out_*.txt`;
$n = 0;
for ($i = 0; $i < (2.0); $i += (1/(24*60))) {
   print "=== $i $n\n";
   $file = sprintf("out_%05d.png", $n);
   $txt = sprintf("out_%05d.txt", $n);
   $n++;
   `./calcdata 34.0007 -81.0348 2022/11/5 $i > $txt; convert -size 1024x1024 -depth 8 RGBA:out.bin $file`;
}
`ffmpeg -framerate 30 -pattern_type glob -i "out*.png" -c:v libx264 out.mp4`;
