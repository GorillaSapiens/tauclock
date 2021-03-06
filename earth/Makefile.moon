all:
	./grid.pl
	convert moon.tif -scale 2560x1280 moon.png
	cp moon.png world.png
	convert world.png world.xpm
	./bitmapish.pl < world.xpm > world.h
	gcc -g world.c -lm
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
