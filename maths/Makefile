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

.PHONY: all
all: trigmain tests fasttests oneday

trigmain: trig1.c trig1_main.c
	gcc -Wall -g -Ofast $^ -lm -o trigmain

tests: astro.c precise.c
	gcc -DVERBOSE -DTEST -Wall -g -Ofast $^ -lm -o tests

fasttests: astro.c precise.c
	gcc -DTEST -Wall -g -Ofast $^ -lm -o fasttests

oneday: astro.c precise.c
	gcc -DONEDAY -Wall -g -Ofast $^ -lm -o oneday

# vim: expandtab:noai:ts=3:sw=3
