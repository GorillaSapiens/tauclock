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

$lat=-85;
$lon=-85;
$step=1/60;
#$step=1/12;
$rate=30;
$start=0;
$length=366;

`rm out*.png out*.txt *.mp4`;
$n = 0;
for ($i = $start; $i < ($length+$start); $i += $step) {
   print "=== $i $n\n";
   $file = sprintf("out_%06d.png", $n);
   $txt = sprintf("out_%06d.txt", $n);
   $on = $n;
   $n++;
   `PROVIDER=$on ./calcdata $lat $lon $i > $txt; convert -size 1024x1024 -depth 8 RGBA:out.bin $file`;
}
`ffmpeg -framerate $rate -pattern_type glob -i "out*.png" -c:v libx264 out.mp4`;
