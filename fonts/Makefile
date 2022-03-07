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

all: .part_a .part_b .part_c .part_d
	touch .bdf

clean:
	rm -rf deja* ico* *.h *.bdf *.png *.zip *.ttf
	rm .bdf .part_a .part_b .part_c .part_d

.part_a:
	wget http://sourceforge.net/projects/dejavu/files/dejavu/2.37/dejavu-fonts-ttf-2.37.tar.bz2
	tar -xjvf deja*.tar.bz2
	-otf2bdf -p 50 dejavu*/ttf/DejaVuSansMono-Bold.ttf -o djsmb_50.bdf
	-otf2bdf -p 20 dejavu*/ttf/DejaVuSansMono-Bold.ttf -o djsmb_20.bdf
	-otf2bdf -p 16 dejavu*/ttf/DejaVuSansMono-Bold.ttf -o djsmb_16.bdf
	-otf2bdf -p 50 dejavu*/ttf/DejaVuSansMono-Oblique.ttf -o djsmo_50.bdf
	-otf2bdf -p 20 dejavu*/ttf/DejaVuSansMono-Oblique.ttf -o djsmo_20.bdf
	-otf2bdf -p 16 dejavu*/ttf/DejaVuSansMono-Oblique.ttf -o djsmo_16.bdf
	touch .part_a

.part_b: png2h.pl
	rm -rf weather-underground-icons
	git clone https://github.com/manifestinteractive/weather-underground-icons.git
	rm -rf icons
	mv weather-underground-icons/dist/icons .
	rm -rf weather-underground-icons
	./png2h.pl icons/solid-white/png/32x32/
	./png2h.pl icons/solid-white/png/64x64/
	./png2h.pl icons/solid-white/png/128x128/
	touch .part_b

.part_c:
	wget https://astrology-symbols.com/font_files/astrodotbasic.zip
	unzip astrodotbasic.zip
	-otf2bdf -p 16 AstroDotBasic.ttf -o astro_16.bdf
	-otf2bdf -p 20 AstroDotBasic.ttf -o astro_20.bdf
	-otf2bdf -p 32 AstroDotBasic.ttf -o astro_32.bdf
	-otf2bdf -p 50 AstroDotBasic.ttf -o astro_50.bdf
	touch .part_c

.part_d:
	find . -name \*.bdf | xargs -L 1 ./bdf2h.pl
