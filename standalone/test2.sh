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
DATE=0
#LD=0
LD=257

rm out*
./calcdata $LAT $LON $DATE 0.000 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_000.png
./calcdata $LAT $LON $DATE 0.005 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_001.png
./calcdata $LAT $LON $DATE 0.010 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_002.png
./calcdata $LAT $LON $DATE 0.015 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_003.png
./calcdata $LAT $LON $DATE 0.020 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_004.png
./calcdata $LAT $LON $DATE 0.025 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_005.png
./calcdata $LAT $LON $DATE 0.030 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_006.png
./calcdata $LAT $LON $DATE 0.035 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_007.png
./calcdata $LAT $LON $DATE 0.040 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_008.png
./calcdata $LAT $LON $DATE 0.045 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_009.png
./calcdata $LAT $LON $DATE 0.050 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_010.png
./calcdata $LAT $LON $DATE 0.055 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_011.png
./calcdata $LAT $LON $DATE 0.060 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_012.png
./calcdata $LAT $LON $DATE 0.065 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_013.png
./calcdata $LAT $LON $DATE 0.070 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_014.png
./calcdata $LAT $LON $DATE 0.075 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_015.png
./calcdata $LAT $LON $DATE 0.080 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_016.png
./calcdata $LAT $LON $DATE 0.085 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_017.png
./calcdata $LAT $LON $DATE 0.090 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_018.png
./calcdata $LAT $LON $DATE 0.095 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_019.png
./calcdata $LAT $LON $DATE 0.100 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_020.png
./calcdata $LAT $LON $DATE 0.105 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_021.png
./calcdata $LAT $LON $DATE 0.110 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_022.png
./calcdata $LAT $LON $DATE 0.115 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_023.png
./calcdata $LAT $LON $DATE 0.120 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_024.png
./calcdata $LAT $LON $DATE 0.125 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_025.png
./calcdata $LAT $LON $DATE 0.130 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_026.png
./calcdata $LAT $LON $DATE 0.135 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_027.png
./calcdata $LAT $LON $DATE 0.140 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_028.png
./calcdata $LAT $LON $DATE 0.145 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_029.png
./calcdata $LAT $LON $DATE 0.150 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_030.png
./calcdata $LAT $LON $DATE 0.155 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_031.png
./calcdata $LAT $LON $DATE 0.160 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_032.png
./calcdata $LAT $LON $DATE 0.165 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_033.png
./calcdata $LAT $LON $DATE 0.170 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_034.png
./calcdata $LAT $LON $DATE 0.175 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_035.png
./calcdata $LAT $LON $DATE 0.180 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_036.png
./calcdata $LAT $LON $DATE 0.185 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_037.png
./calcdata $LAT $LON $DATE 0.190 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_038.png
./calcdata $LAT $LON $DATE 0.195 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_039.png
./calcdata $LAT $LON $DATE 0.200 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_040.png
./calcdata $LAT $LON $DATE 0.205 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_041.png
./calcdata $LAT $LON $DATE 0.210 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_042.png
./calcdata $LAT $LON $DATE 0.215 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_043.png
./calcdata $LAT $LON $DATE 0.220 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_044.png
./calcdata $LAT $LON $DATE 0.225 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_045.png
./calcdata $LAT $LON $DATE 0.230 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_046.png
./calcdata $LAT $LON $DATE 0.235 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_047.png
./calcdata $LAT $LON $DATE 0.240 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_048.png
./calcdata $LAT $LON $DATE 0.245 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_049.png
./calcdata $LAT $LON $DATE 0.250 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_050.png
./calcdata $LAT $LON $DATE 0.255 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_051.png
./calcdata $LAT $LON $DATE 0.260 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_052.png
./calcdata $LAT $LON $DATE 0.265 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_053.png
./calcdata $LAT $LON $DATE 0.270 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_054.png
./calcdata $LAT $LON $DATE 0.275 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_055.png
./calcdata $LAT $LON $DATE 0.280 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_056.png
./calcdata $LAT $LON $DATE 0.285 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_057.png
./calcdata $LAT $LON $DATE 0.290 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_058.png
./calcdata $LAT $LON $DATE 0.295 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_059.png
./calcdata $LAT $LON $DATE 0.300 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_060.png
./calcdata $LAT $LON $DATE 0.305 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_061.png
./calcdata $LAT $LON $DATE 0.310 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_062.png
./calcdata $LAT $LON $DATE 0.315 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_063.png
./calcdata $LAT $LON $DATE 0.320 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_064.png
./calcdata $LAT $LON $DATE 0.325 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_065.png
./calcdata $LAT $LON $DATE 0.330 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_066.png
./calcdata $LAT $LON $DATE 0.335 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_067.png
./calcdata $LAT $LON $DATE 0.340 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_068.png
./calcdata $LAT $LON $DATE 0.345 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_069.png
./calcdata $LAT $LON $DATE 0.350 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_070.png
./calcdata $LAT $LON $DATE 0.355 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_071.png
./calcdata $LAT $LON $DATE 0.360 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_072.png
./calcdata $LAT $LON $DATE 0.365 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_073.png
./calcdata $LAT $LON $DATE 0.370 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_074.png
./calcdata $LAT $LON $DATE 0.375 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_075.png
./calcdata $LAT $LON $DATE 0.380 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_076.png
./calcdata $LAT $LON $DATE 0.385 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_077.png
./calcdata $LAT $LON $DATE 0.390 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_078.png
./calcdata $LAT $LON $DATE 0.395 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_079.png
./calcdata $LAT $LON $DATE 0.400 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_080.png
./calcdata $LAT $LON $DATE 0.405 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_081.png
./calcdata $LAT $LON $DATE 0.410 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_082.png
./calcdata $LAT $LON $DATE 0.415 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_083.png
./calcdata $LAT $LON $DATE 0.420 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_084.png
./calcdata $LAT $LON $DATE 0.425 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_085.png
./calcdata $LAT $LON $DATE 0.430 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_086.png
./calcdata $LAT $LON $DATE 0.435 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_087.png
./calcdata $LAT $LON $DATE 0.440 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_088.png
./calcdata $LAT $LON $DATE 0.445 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_089.png
./calcdata $LAT $LON $DATE 0.450 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_090.png
./calcdata $LAT $LON $DATE 0.455 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_091.png
./calcdata $LAT $LON $DATE 0.460 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_092.png
./calcdata $LAT $LON $DATE 0.465 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_093.png
./calcdata $LAT $LON $DATE 0.470 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_094.png
./calcdata $LAT $LON $DATE 0.475 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_095.png
./calcdata $LAT $LON $DATE 0.480 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_096.png
./calcdata $LAT $LON $DATE 0.485 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_097.png
./calcdata $LAT $LON $DATE 0.490 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_098.png
./calcdata $LAT $LON $DATE 0.495 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_099.png
./calcdata $LAT $LON $DATE 0.500 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_100.png
./calcdata $LAT $LON $DATE 0.505 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_101.png
./calcdata $LAT $LON $DATE 0.510 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_102.png
./calcdata $LAT $LON $DATE 0.515 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_103.png
./calcdata $LAT $LON $DATE 0.520 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_104.png
./calcdata $LAT $LON $DATE 0.525 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_105.png
./calcdata $LAT $LON $DATE 0.530 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_106.png
./calcdata $LAT $LON $DATE 0.535 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_107.png
./calcdata $LAT $LON $DATE 0.540 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_108.png
./calcdata $LAT $LON $DATE 0.545 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_109.png
./calcdata $LAT $LON $DATE 0.550 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_110.png
./calcdata $LAT $LON $DATE 0.555 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_111.png
./calcdata $LAT $LON $DATE 0.560 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_112.png
./calcdata $LAT $LON $DATE 0.565 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_113.png
./calcdata $LAT $LON $DATE 0.570 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_114.png
./calcdata $LAT $LON $DATE 0.575 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_115.png
./calcdata $LAT $LON $DATE 0.580 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_116.png
./calcdata $LAT $LON $DATE 0.585 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_117.png
./calcdata $LAT $LON $DATE 0.590 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_118.png
./calcdata $LAT $LON $DATE 0.595 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_119.png
./calcdata $LAT $LON $DATE 0.600 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_120.png
./calcdata $LAT $LON $DATE 0.605 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_121.png
./calcdata $LAT $LON $DATE 0.610 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_122.png
./calcdata $LAT $LON $DATE 0.615 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_123.png
./calcdata $LAT $LON $DATE 0.620 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_124.png
./calcdata $LAT $LON $DATE 0.625 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_125.png
./calcdata $LAT $LON $DATE 0.630 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_126.png
./calcdata $LAT $LON $DATE 0.635 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_127.png
./calcdata $LAT $LON $DATE 0.640 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_128.png
./calcdata $LAT $LON $DATE 0.645 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_129.png
./calcdata $LAT $LON $DATE 0.650 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_130.png
./calcdata $LAT $LON $DATE 0.655 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_131.png
./calcdata $LAT $LON $DATE 0.660 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_132.png
./calcdata $LAT $LON $DATE 0.665 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_133.png
./calcdata $LAT $LON $DATE 0.670 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_134.png
./calcdata $LAT $LON $DATE 0.675 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_135.png
./calcdata $LAT $LON $DATE 0.680 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_136.png
./calcdata $LAT $LON $DATE 0.685 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_137.png
./calcdata $LAT $LON $DATE 0.690 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_138.png
./calcdata $LAT $LON $DATE 0.695 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_139.png
./calcdata $LAT $LON $DATE 0.700 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_140.png
./calcdata $LAT $LON $DATE 0.705 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_141.png
./calcdata $LAT $LON $DATE 0.710 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_142.png
./calcdata $LAT $LON $DATE 0.715 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_143.png
./calcdata $LAT $LON $DATE 0.720 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_144.png
./calcdata $LAT $LON $DATE 0.725 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_145.png
./calcdata $LAT $LON $DATE 0.730 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_146.png
./calcdata $LAT $LON $DATE 0.735 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_147.png
./calcdata $LAT $LON $DATE 0.740 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_148.png
./calcdata $LAT $LON $DATE 0.745 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_149.png
./calcdata $LAT $LON $DATE 0.750 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_150.png
./calcdata $LAT $LON $DATE 0.755 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_151.png
./calcdata $LAT $LON $DATE 0.760 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_152.png
./calcdata $LAT $LON $DATE 0.765 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_153.png
./calcdata $LAT $LON $DATE 0.770 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_154.png
./calcdata $LAT $LON $DATE 0.775 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_155.png
./calcdata $LAT $LON $DATE 0.780 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_156.png
./calcdata $LAT $LON $DATE 0.785 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_157.png
./calcdata $LAT $LON $DATE 0.790 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_158.png
./calcdata $LAT $LON $DATE 0.795 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_159.png
./calcdata $LAT $LON $DATE 0.800 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_160.png
./calcdata $LAT $LON $DATE 0.805 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_161.png
./calcdata $LAT $LON $DATE 0.810 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_162.png
./calcdata $LAT $LON $DATE 0.815 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_163.png
./calcdata $LAT $LON $DATE 0.820 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_164.png
./calcdata $LAT $LON $DATE 0.825 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_165.png
./calcdata $LAT $LON $DATE 0.830 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_166.png
./calcdata $LAT $LON $DATE 0.835 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_167.png
./calcdata $LAT $LON $DATE 0.840 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_168.png
./calcdata $LAT $LON $DATE 0.845 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_169.png
./calcdata $LAT $LON $DATE 0.850 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_170.png
./calcdata $LAT $LON $DATE 0.855 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_171.png
./calcdata $LAT $LON $DATE 0.860 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_172.png
./calcdata $LAT $LON $DATE 0.865 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_173.png
./calcdata $LAT $LON $DATE 0.870 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_174.png
./calcdata $LAT $LON $DATE 0.875 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_175.png
./calcdata $LAT $LON $DATE 0.880 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_176.png
./calcdata $LAT $LON $DATE 0.885 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_177.png
./calcdata $LAT $LON $DATE 0.890 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_178.png
./calcdata $LAT $LON $DATE 0.895 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_179.png
./calcdata $LAT $LON $DATE 0.900 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_180.png
./calcdata $LAT $LON $DATE 0.905 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_181.png
./calcdata $LAT $LON $DATE 0.910 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_182.png
./calcdata $LAT $LON $DATE 0.915 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_183.png
./calcdata $LAT $LON $DATE 0.920 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_184.png
./calcdata $LAT $LON $DATE 0.925 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_185.png
./calcdata $LAT $LON $DATE 0.930 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_186.png
./calcdata $LAT $LON $DATE 0.935 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_187.png
./calcdata $LAT $LON $DATE 0.940 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_188.png
./calcdata $LAT $LON $DATE 0.945 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_189.png
./calcdata $LAT $LON $DATE 0.950 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_190.png
./calcdata $LAT $LON $DATE 0.955 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_191.png
./calcdata $LAT $LON $DATE 0.960 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_192.png
./calcdata $LAT $LON $DATE 0.965 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_193.png
./calcdata $LAT $LON $DATE 0.970 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_194.png
./calcdata $LAT $LON $DATE 0.975 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_195.png
./calcdata $LAT $LON $DATE 0.980 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_196.png
./calcdata $LAT $LON $DATE 0.985 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_197.png
./calcdata $LAT $LON $DATE 0.990 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_198.png
./calcdata $LAT $LON $DATE 0.995 $LD;convert -size 1024x1024 -depth 8 RGBA:out.bin out_199.png
convert -delay 20 out*.png out.gif
