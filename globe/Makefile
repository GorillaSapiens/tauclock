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

all:
	./grid.pl
	convert -colorspace RGB -depth 8 outlines.png -morphology erode disk:5 -channel RGB -negate thickoutlines.png
	composite thickoutlines.png grid.png  globe2.png
	composite globe2.png globe_top_bathy_bigger.png globe.png
	convert globe.png globe.xpm
	./bitmapish.pl < globe.xpm > globei.h
	gcc -g -DSTANDALONE -I../maths -I../standalone ../maths/trig1.c ../standalone/draw.c globe.c -lm
	./a.out 512 0 0 0 > zero.raw
	convert -size 512x512 -depth 8 RGBA:zero.raw -scale 1024x1024 zero.png
	./a.out 512 90 0 0 > x.raw
	convert -size 512x512 -depth 8 RGBA:x.raw -scale 1024x1024 x.png
	./a.out 512 0 90 0 > y.raw
	convert -size 512x512 -depth 8 RGBA:y.raw -scale 1024x1024 y.png
	./a.out 512 0 0 90 > z.raw
	convert -size 512x512 -depth 8 RGBA:z.raw -scale 1024x1024 z.png
	./a.out 512 -34 -58 0 > buenos.raw
	convert -size 512x512 -depth 8 RGBA:buenos.raw -scale 1024x1024 buenos.png
	./a.out 512 34 -81 0 > columbia.raw
	convert -size 512x512 -depth 8 RGBA:columbia.raw -scale 1024x1024 columbia.png

# broken, maybe can fix some day
# small:
# 	convert globe_top_bathy_bigger.png globe.xpm
# 	./bitmapish.pl < globe.xpm > globe.h
# 	gcc -g globe.c -lm
# 	./a.out 64 34 -81 0 > csmall.raw
# 	convert -size 64x64 -depth 8 RGB:csmall.raw csmall.png

# broken, maybe can fix some day
# moon:
# 	convert moon.tif -scale 2560x1280\! moon.xpm
# 	./bitmapish.pl < moon.xpm > globe.h
# 	gcc -g globe.c -lm
# 	./a.out 512 0 0 0 > moon.raw
# 	convert -size 512x512 -depth 8 RGB:moon.raw -scale 1024x1024 moon.png

clean:
	rm -f a.out
	rm -f buenos.png buenos.raw
	rm -f columbia.png columbia.raw
	rm -f globe.png globe.xpm globe2.png grid.png thickoutlines.png
	rm -f x.png x.raw
	rm -f y.png y.raw
	rm -f z.png z.raw
	rm -f zero.png zero.raw

