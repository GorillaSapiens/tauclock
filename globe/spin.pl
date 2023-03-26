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

$oii = "000";

for ($i = 0; $i < 360; $i++) {
   $j = 360 - ($i - 180);
   print "$j\n";
   $ii = sprintf("%03d", $i);
   print `time ./a.out 512 0 $j 0 > /tmp/spin_$ii.raw`;
   $oii = $ii;
}

`convert -delay 5 -loop 0 -size 512x512 -depth 8 RGB:/tmp/spin_*.raw spin.gif`;
