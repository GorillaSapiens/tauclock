.PHONY: all
all: trigmain tests fasttests oneday

trigmain: trig1.c trig1_main.c
	gcc -g -Ofast $^ -lm -o trigmain

tests: astro.c trig1.c
	gcc -DVERBOSE -DTEST -g -Ofast $^ -o tests

fasttests: astro.c trig1.c
	gcc -DTEST -g -Ofast $^ -o fasttests

oneday: astro.c trig1.c
	gcc -DONEDAY -g -Ofast $^ -o oneday
