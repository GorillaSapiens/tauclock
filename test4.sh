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

rm *.png
./calcdata 30 -80 0;convert -size 1024x1024 -depth 8 RGBA:out.bin out_00.png
./calcdata 30 -80 1;convert -size 1024x1024 -depth 8 RGBA:out.bin out_01.png
./calcdata 30 -80 2;convert -size 1024x1024 -depth 8 RGBA:out.bin out_02.png
./calcdata 30 -80 3;convert -size 1024x1024 -depth 8 RGBA:out.bin out_03.png
./calcdata 30 -80 4;convert -size 1024x1024 -depth 8 RGBA:out.bin out_04.png
./calcdata 30 -80 5;convert -size 1024x1024 -depth 8 RGBA:out.bin out_05.png
./calcdata 30 -80 6;convert -size 1024x1024 -depth 8 RGBA:out.bin out_06.png
./calcdata 30 -80 7;convert -size 1024x1024 -depth 8 RGBA:out.bin out_07.png
./calcdata 30 -80 8;convert -size 1024x1024 -depth 8 RGBA:out.bin out_08.png
./calcdata 30 -80 9;convert -size 1024x1024 -depth 8 RGBA:out.bin out_09.png
./calcdata 30 -80 10;convert -size 1024x1024 -depth 8 RGBA:out.bin out_10.png
./calcdata 30 -80 11;convert -size 1024x1024 -depth 8 RGBA:out.bin out_11.png
./calcdata 30 -80 12;convert -size 1024x1024 -depth 8 RGBA:out.bin out_12.png
./calcdata 30 -80 13;convert -size 1024x1024 -depth 8 RGBA:out.bin out_13.png
./calcdata 30 -80 14;convert -size 1024x1024 -depth 8 RGBA:out.bin out_14.png
./calcdata 30 -80 15;convert -size 1024x1024 -depth 8 RGBA:out.bin out_15.png
./calcdata 30 -80 16;convert -size 1024x1024 -depth 8 RGBA:out.bin out_16.png
./calcdata 30 -80 17;convert -size 1024x1024 -depth 8 RGBA:out.bin out_17.png
./calcdata 30 -80 18;convert -size 1024x1024 -depth 8 RGBA:out.bin out_18.png
./calcdata 30 -80 19;convert -size 1024x1024 -depth 8 RGBA:out.bin out_19.png
./calcdata 30 -80 20;convert -size 1024x1024 -depth 8 RGBA:out.bin out_20.png
./calcdata 30 -80 21;convert -size 1024x1024 -depth 8 RGBA:out.bin out_21.png
./calcdata 30 -80 22;convert -size 1024x1024 -depth 8 RGBA:out.bin out_22.png
./calcdata 30 -80 23;convert -size 1024x1024 -depth 8 RGBA:out.bin out_23.png
./calcdata 30 -80 24;convert -size 1024x1024 -depth 8 RGBA:out.bin out_24.png
./calcdata 30 -80 25;convert -size 1024x1024 -depth 8 RGBA:out.bin out_25.png
./calcdata 30 -80 26;convert -size 1024x1024 -depth 8 RGBA:out.bin out_26.png
./calcdata 30 -80 27;convert -size 1024x1024 -depth 8 RGBA:out.bin out_27.png
./calcdata 30 -80 28;convert -size 1024x1024 -depth 8 RGBA:out.bin out_28.png
./calcdata 30 -80 29;convert -size 1024x1024 -depth 8 RGBA:out.bin out_29.png
convert -delay 100 *.png out.gif
