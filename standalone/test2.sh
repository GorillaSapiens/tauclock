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

LAT=30
LON=-80
#LD=0
LD=257

rm out*
./calcdata $LAT $LON 0 0.000 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_000.png
./calcdata $LAT $LON 0 0.015 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_001.png
./calcdata $LAT $LON 0 0.030 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_002.png
./calcdata $LAT $LON 0 0.045 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_003.png
./calcdata $LAT $LON 0 0.060 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_004.png
./calcdata $LAT $LON 0 0.075 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_005.png
./calcdata $LAT $LON 0 0.090 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_006.png
./calcdata $LAT $LON 0 0.105 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_007.png
./calcdata $LAT $LON 0 0.120 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_008.png
./calcdata $LAT $LON 0 0.135 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_009.png
./calcdata $LAT $LON 0 0.150 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_010.png
./calcdata $LAT $LON 0 0.165 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_011.png
./calcdata $LAT $LON 0 0.180 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_012.png
./calcdata $LAT $LON 0 0.195 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_013.png
./calcdata $LAT $LON 0 0.210 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_014.png
./calcdata $LAT $LON 0 0.225 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_015.png
./calcdata $LAT $LON 0 0.240 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_016.png
./calcdata $LAT $LON 0 0.255 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_017.png
./calcdata $LAT $LON 0 0.270 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_018.png
./calcdata $LAT $LON 0 0.285 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_019.png
./calcdata $LAT $LON 0 0.300 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_020.png
./calcdata $LAT $LON 0 0.315 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_021.png
./calcdata $LAT $LON 0 0.330 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_022.png
./calcdata $LAT $LON 0 0.345 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_023.png
./calcdata $LAT $LON 0 0.360 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_024.png
./calcdata $LAT $LON 0 0.375 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_025.png
./calcdata $LAT $LON 0 0.390 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_026.png
./calcdata $LAT $LON 0 0.405 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_027.png
./calcdata $LAT $LON 0 0.420 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_028.png
./calcdata $LAT $LON 0 0.435 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_029.png
./calcdata $LAT $LON 0 0.450 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_030.png
./calcdata $LAT $LON 0 0.465 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_031.png
./calcdata $LAT $LON 0 0.480 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_032.png
./calcdata $LAT $LON 0 0.495 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_033.png
./calcdata $LAT $LON 0 0.510 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_034.png
./calcdata $LAT $LON 0 0.525 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_035.png
./calcdata $LAT $LON 0 0.540 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_036.png
./calcdata $LAT $LON 0 0.555 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_037.png
./calcdata $LAT $LON 0 0.570 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_038.png
./calcdata $LAT $LON 0 0.585 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_039.png
./calcdata $LAT $LON 0 0.600 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_040.png
./calcdata $LAT $LON 0 0.615 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_041.png
./calcdata $LAT $LON 0 0.630 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_042.png
./calcdata $LAT $LON 0 0.645 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_043.png
./calcdata $LAT $LON 0 0.660 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_044.png
./calcdata $LAT $LON 0 0.675 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_045.png
./calcdata $LAT $LON 0 0.690 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_046.png
./calcdata $LAT $LON 0 0.705 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_047.png
./calcdata $LAT $LON 0 0.720 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_048.png
./calcdata $LAT $LON 0 0.735 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_049.png
./calcdata $LAT $LON 0 0.750 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_050.png
./calcdata $LAT $LON 0 0.765 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_051.png
./calcdata $LAT $LON 0 0.780 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_052.png
./calcdata $LAT $LON 0 0.795 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_053.png
./calcdata $LAT $LON 0 0.810 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_054.png
./calcdata $LAT $LON 0 0.825 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_055.png
./calcdata $LAT $LON 0 0.840 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_056.png
./calcdata $LAT $LON 0 0.855 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_057.png
./calcdata $LAT $LON 0 0.870 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_058.png
./calcdata $LAT $LON 0 0.885 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_059.png
./calcdata $LAT $LON 0 0.900 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_060.png
./calcdata $LAT $LON 0 0.915 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_061.png
./calcdata $LAT $LON 0 0.930 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_062.png
./calcdata $LAT $LON 0 0.945 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_063.png
./calcdata $LAT $LON 0 0.960 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_064.png
./calcdata $LAT $LON 0 0.975 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_065.png
./calcdata $LAT $LON 0 0.990 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin -alpha off out_066.png
convert -delay 100 out*.png out.gif
