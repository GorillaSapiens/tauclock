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

COLOR=-fdiagnostics-color=always
#COLOR=

all: fonts/.bdf calcdata calcdataprecise calcrange html/index.html

html/index.html: doxygen.conf clock.c draw.c ../maths/trig1.c ../maths/astro.c
	doxygen doxygen.conf 1>/dev/null

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

calcdata: main.c draw.c clock.c ../maths/trig1.c ../maths/astro.c
	gcc $(COLOR) -flarge-source-files -g -pg -Wall -DSTANDALONE -I../maths $^ -lm -o calcdata
#gcc $(COLOR) -g -pg -Ofast -Wall -DSTANDALONE $^ -lm -o calcdata

calcdataprecise: main.c draw.c clock.c precise.c ../maths/astro.c
	gcc $(COLOR) -flarge-source-files -g -pg -Wall -DSTANDALONE -I../maths $^ -lm -o calcdataprecise

calcrange: mainrange.c draw.c clock.c ../maths/trig1.c ../maths/astro.c
	gcc $(COLOR) -flarge-source-files -g -pg -Ofast -Wall -DSTANDALONE -I../maths $^ -lm -o calcrange

tidy:
	rm -f out* *~

clean: tidy
	rm -f calcdata calcrange
	rm -rf latex html
	( cd fonts ; make clean )

squeaky:
	echo done

# vim: expandtab:noai:ts=3:sw=3
