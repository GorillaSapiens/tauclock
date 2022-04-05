# goddess help us, we're going to convert the ENTIRE libnova library
# to a single kotlin file.

all: libnova.c

libnova.c: libnova.txt
	./dedup.pl < libnova.txt > libnova.c

libnova.txt: libnova-0.15.0
	(cd libnova-0.15.0/src; \
         for each in *.c; \
         do \
         cpp -I. --sysroot ../../fakeroot $$each; \
         done ) | sed "s/^#.*//g" > libnova.txt

libnova-0.15.0: libnova-0.15.0.tar.gz
	tar -xzvf libnova-0.15.0.tar.gz

libnova-0.15.0.tar.gz:
	wget https://sourceforge.net/projects/libnova/files/libnova/v%200.15.0/libnova-0.15.0.tar.gz/download -O libnova-0.15.0.tar.gz

