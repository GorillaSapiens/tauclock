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

rm out*.png
./calcdata 30 -80 .00;convert -size 1024x1024 -depth 8 RGBA:out.bin out_00.png
./calcdata 30 -80 .05;convert -size 1024x1024 -depth 8 RGBA:out.bin out_01.png
./calcdata 30 -80 .10;convert -size 1024x1024 -depth 8 RGBA:out.bin out_02.png
./calcdata 30 -80 .15;convert -size 1024x1024 -depth 8 RGBA:out.bin out_03.png
./calcdata 30 -80 .20;convert -size 1024x1024 -depth 8 RGBA:out.bin out_04.png
./calcdata 30 -80 .25;convert -size 1024x1024 -depth 8 RGBA:out.bin out_05.png
./calcdata 30 -80 .30;convert -size 1024x1024 -depth 8 RGBA:out.bin out_06.png
./calcdata 30 -80 .35;convert -size 1024x1024 -depth 8 RGBA:out.bin out_07.png
./calcdata 30 -80 .40;convert -size 1024x1024 -depth 8 RGBA:out.bin out_08.png
./calcdata 30 -80 .45;convert -size 1024x1024 -depth 8 RGBA:out.bin out_09.png
./calcdata 30 -80 .50;convert -size 1024x1024 -depth 8 RGBA:out.bin out_10.png
./calcdata 30 -80 .55;convert -size 1024x1024 -depth 8 RGBA:out.bin out_11.png
./calcdata 30 -80 .60;convert -size 1024x1024 -depth 8 RGBA:out.bin out_12.png
./calcdata 30 -80 .65;convert -size 1024x1024 -depth 8 RGBA:out.bin out_13.png
./calcdata 30 -80 .70;convert -size 1024x1024 -depth 8 RGBA:out.bin out_14.png
./calcdata 30 -80 .75;convert -size 1024x1024 -depth 8 RGBA:out.bin out_15.png
./calcdata 30 -80 .80;convert -size 1024x1024 -depth 8 RGBA:out.bin out_16.png
./calcdata 30 -80 .85;convert -size 1024x1024 -depth 8 RGBA:out.bin out_17.png
./calcdata 30 -80 .90;convert -size 1024x1024 -depth 8 RGBA:out.bin out_18.png
./calcdata 30 -80 .95;convert -size 1024x1024 -depth 8 RGBA:out.bin out_19.png
convert -delay 100 *.png out.gif
