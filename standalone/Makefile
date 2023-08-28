# vim: expandtab:noai:ts=3:sw=3

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

#OPTIM=-g -pg -Ofast
OPTIM=-g -Og -ggdb

all: fonts.h calcdata calcrange html/index.html

html/index.html: doxygen.conf clock.c draw.c ../maths/astro.c
	doxygen doxygen.conf 1>/dev/null

indent:
	indent -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -nce -ci3 \
		-cli3 -d0 -di1 -nfc1 -i3 -ip0 -l80 -lp -npcs -nprs -npsl -sai \
		-saf -saw -ncs -nsc -nut -sob -nfca -cp33 -ss -ts8 -il1 -brf \
		*.[ch]

fonts.h: fonts/Makefile
	(cd fonts ; make)

icons:
	git clone https://github.com/manifestinteractive/weather-underground-icons.git
	mv weather-underground-icons/dist/icons .
	rm -rf weather-underground-icons

calcdata: main.o draw.o schrift.o clock.o ../maths/astro.o ../maths/precise.o
	gcc \
		$(COLOR) \
		$(OPTIM) -Wall \
		$^ -lm -o calcdata

calcrange: mainrange.o draw.o schrift.o clock.o ../maths/astro.o ../maths/precise.o
	gcc \
		$(COLOR) \
		$(OPTIM) -Wall \
		$^ -lm -o calcrange

%.o: %.c
	gcc -c \
		$(COLOR) \
		-flarge-source-files \
		$(OPTIM) -Wall -DSTANDALONE \
		-I../maths $*.c -o $*.o

depend:
	makedepend -DSTANDALONE -I../maths main.c draw.c clock.c ../maths/astro.c

tidy:
	rm -f out* *~

clean: tidy
	rm -f calcdata calcrange
	rm -rf latex html
	( cd fonts ; make clean )

squeaky:
	echo done

# DO NOT DELETE

main.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/time.h
main.o: /usr/include/features.h /usr/include/features-time64.h
main.o: /usr/include/stdc-predef.h /usr/include/string.h
main.o: /usr/include/strings.h draw.h clock.h
draw.o: /usr/include/math.h /usr/include/stdio.h /usr/include/string.h
draw.o: /usr/include/strings.h /usr/include/features.h
draw.o: /usr/include/features-time64.h /usr/include/stdc-predef.h
draw.o: /usr/include/stdlib.h /usr/include/fcntl.h /usr/include/unistd.h
draw.o: draw.h ../maths/precise.h
clock.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
clock.o: /usr/include/strings.h /usr/include/features.h
clock.o: /usr/include/features-time64.h /usr/include/stdc-predef.h
clock.o: /usr/include/unistd.h /usr/include/fcntl.h /usr/include/math.h
clock.o: /usr/include/locale.h /usr/include/assert.h ../maths/precise.h
clock.o: ../maths/astro.h /usr/include/time.h draw.h moon_xpm.h
../maths/astro.o: /usr/include/time.h /usr/include/features.h
../maths/astro.o: /usr/include/features-time64.h /usr/include/stdc-predef.h
../maths/astro.o: ../maths/precise.h /usr/include/math.h ../maths/astro.h
