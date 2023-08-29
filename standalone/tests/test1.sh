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

rm out*
./calcdata 34.0007 -81.0348 0  > out00.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_00.png
./calcdata 34.0007 -81.0348 1  > out01.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_01.png
./calcdata 34.0007 -81.0348 2  > out02.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_02.png
./calcdata 34.0007 -81.0348 3  > out03.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_03.png
./calcdata 34.0007 -81.0348 4  > out04.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_04.png
./calcdata 34.0007 -81.0348 5  > out05.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_05.png
./calcdata 34.0007 -81.0348 6  > out06.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_06.png
./calcdata 34.0007 -81.0348 7  > out07.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_07.png
./calcdata 34.0007 -81.0348 8  > out08.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_08.png
./calcdata 34.0007 -81.0348 9  > out09.txt; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_09.png
./calcdata 34.0007 -81.0348 10 > out10.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_10.png
./calcdata 34.0007 -81.0348 11 > out11.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_11.png
./calcdata 34.0007 -81.0348 12 > out12.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_12.png
./calcdata 34.0007 -81.0348 13 > out13.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_13.png
./calcdata 34.0007 -81.0348 14 > out14.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_14.png
./calcdata 34.0007 -81.0348 15 > out15.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_15.png
./calcdata 34.0007 -81.0348 16 > out16.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_16.png
./calcdata 34.0007 -81.0348 17 > out17.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_17.png
./calcdata 34.0007 -81.0348 18 > out18.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_18.png
./calcdata 34.0007 -81.0348 19 > out19.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_19.png
./calcdata 34.0007 -81.0348 20 > out20.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_20.png
./calcdata 34.0007 -81.0348 21 > out21.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_21.png
./calcdata 34.0007 -81.0348 22 > out22.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_22.png
./calcdata 34.0007 -81.0348 23 > out23.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_23.png
./calcdata 34.0007 -81.0348 24 > out24.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_24.png
./calcdata 34.0007 -81.0348 25 > out25.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_25.png
./calcdata 34.0007 -81.0348 26 > out26.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_26.png
./calcdata 34.0007 -81.0348 27 > out27.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_27.png
./calcdata 34.0007 -81.0348 28 > out28.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_28.png
./calcdata 34.0007 -81.0348 29 > out29.txt ; convert -size 1024x1024 -depth 8 RGBA:out.bin out.png ; mv out.png out_29.png
convert -delay 100 out*.png out.gif
