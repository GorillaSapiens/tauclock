all:
	gcc precompute.c -lm -o b.out
	./b.out > precomputed.h
	echo "{0,0}" >> precomputed.h
	gcc spin.c -lm
	./a.out 0 0 0 > zero.raw
	convert -size 256x256 -depth 8 RGB:zero.raw -scale 1024x1024 zero.png
	./a.out 1 0 0 > pitch.raw
	convert -size 256x256 -depth 8 RGB:pitch.raw -scale 1024x1024 pitch.png
	./a.out 0 1 0 > roll.raw
	convert -size 256x256 -depth 8 RGB:roll.raw -scale 1024x1024 roll.png
	./a.out 0 0 1 > yaw.raw
	convert -size 256x256 -depth 8 RGB:yaw.raw -scale 1024x1024 yaw.png
