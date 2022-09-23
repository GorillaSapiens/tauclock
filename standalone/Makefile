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

all: fonts/.bdf calcdata libnova-0.15.0/doc/html doxygen

doxygen:
	doxygen doxygen.conf

indent:
	indent -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -nce -ci3 \
		-cli3 -d0 -di1 -nfc1 -i3 -ip0 -l80 -lp -npcs -nprs -npsl -sai \
	        -saf -saw -ncs -nsc -nut -sob -nfca -cp33 -ss -ts8 -il1 -brf \
	        *.[ch]

fonts/.bdf: fonts/Makefile
	(cd fonts ; make)

icons:
	git clone https://github.com/manifestinteractive/weather-underground-icons.git
	mv weather-underground-icons/dist/icons .
	rm -rf weather-underground-icons

calcdata: main.c draw.c clock.c libnova.a
	gcc -g -Wall -DSTANDALONE draw.c clock.c main.c libnova.a -I libnova-0.15.0/src -lm -o calcdata
#gcc -g -Ofast -Wall -DSTANDALONE draw.c clock.c main.c libnova.a -I libnova-0.15.0/src -lm -o calcdata

libnova-0.15.0/doc/html:
	(cd libnova-0.15.0/doc; doxygen doxyfile.in )

libnova.a: libnova-0.15.0
	(cd libnova-0.15.0/src; gcc -Ofast -c *.c -I.)
	ar r libnova.a libnova-0.15.0/src/*.o

libnova-0.15.0: libnova-0.15.0.tar.gz
	tar -xzvf libnova-0.15.0.tar.gz

libnova-0.15.0.tar.gz:
	wget https://sourceforge.net/projects/libnova/files/libnova/v%200.15.0/libnova-0.15.0.tar.gz/download -O libnova-0.15.0.tar.gz

tidy:
	rm -f out* *~

clean: tidy
	rm -f libnova.a doxygen.conf.bak calcdata
	rm -rf libnova-0.15.0 latex html
	( cd fonts ; make clean )
