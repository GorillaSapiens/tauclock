all:
	./grid.pl
	convert -colorspace RGB -depth 8 outlines.png -morphology erode disk:5 -channel RGB -negate thickoutlines.png
	composite thickoutlines.png grid.png  globe2.png
	composite globe2.png globe_top_bathy_bigger.png globe.png
	convert globe.png globe.xpm
	./bitmapish.pl < globe.xpm > globei.h
	gcc -g globe.c -lm
	./a.out 512 0 0 0 > zero.raw
	convert -size 512x512 -depth 8 RGB:zero.raw -scale 1024x1024 zero.png
	./a.out 512 90 0 0 > x.raw
	convert -size 512x512 -depth 8 RGB:x.raw -scale 1024x1024 x.png
	./a.out 512 0 90 0 > y.raw
	convert -size 512x512 -depth 8 RGB:y.raw -scale 1024x1024 y.png
	./a.out 512 0 0 90 > z.raw
	convert -size 512x512 -depth 8 RGB:z.raw -scale 1024x1024 z.png
	./a.out 512 -34 -58 0 > buenos.raw
	convert -size 512x512 -depth 8 RGB:buenos.raw -scale 1024x1024 buenos.png
	./a.out 512 34 -81 0 > columbia.raw
	convert -size 512x512 -depth 8 RGB:columbia.raw -scale 1024x1024 columbia.png

small:
	convert globe_top_bathy_bigger.png globe.xpm
	./bitmapish.pl < globe.xpm > globe.h
	gcc -g globe.c -lm
	./a.out 64 34 -81 0 > csmall.raw
	convert -size 64x64 -depth 8 RGB:csmall.raw csmall.png

moon:
	convert moon.tif -scale 2560x1280\! moon.xpm
	./bitmapish.pl < moon.xpm > globe.h
	gcc -g globe.c -lm
	./a.out 512 0 0 0 > moon.raw
	convert -size 512x512 -depth 8 RGB:moon.raw -scale 1024x1024 moon.png

clean:
	rm -f a.out
	rm -f buenos.png buenos.raw
	rm -f columbia.png columbia.raw
	rm -f globe.png globe.xpm globe2.png grid.png thickoutlines.png
	rm -f x.png x.raw
	rm -f y.png y.raw
	rm -f z.png z.raw
	rm -f zero.png zero.raw

