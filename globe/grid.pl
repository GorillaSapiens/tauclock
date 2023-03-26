#!/usr/bin/perl

# Sunclock, draw a clock with local solar and lunar information
# Copyright (C) 2022,2023 Adam Wozniak / GorillaSapiens
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

$cmd  = "convert -size 2560x1280 xc:transparent";
$cmd .= " -fill CornflowerBlue -stroke CornflowerBlue -strokewidth 3";

for ($i = 0; $i < 36; $i++) {
$x = int(2560 * $i / 36);
$cmd .= " -draw \"line $x,0 $x,1279\"";
}

for ($i = 0; $i < 18; $i++) {
$y = int(1280 * $i / 18);
$cmd .= " -draw \"line 0,$y 2559,$y\"";
}

# tropic = equator +/- 23.5 degrees
# circle = pole +/- 23.5 degrees

$offset = int(1280 * 23.5 / 180);

$cmd .= " -fill LightSteelBlue -stroke LightSteelBlue -strokewidth 3";
$y = $offset;
$cmd .= " -draw \"line 0,$y 2559,$y\"";
$y = 1280 / 2 + $offset;
$cmd .= " -draw \"line 0,$y 2559,$y\"";
$y = 1280 / 2 - $offset;
$cmd .= " -draw \"line 0,$y 2559,$y\"";
$y = 1280 - $offset;
$cmd .= " -draw \"line 0,$y 2559,$y\"";

$cmd .= " -fill LemonChiffon -stroke LemonChiffon -strokewidth 3";
$cmd .= " -draw \"line 1280,0 1280,1279\"";
$cmd .= " -draw \"line 0,640 2559,640\"";
$cmd .= " grid.png";

print `$cmd`;
