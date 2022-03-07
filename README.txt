Sunclock

a clock for camping in the desert

== Compiling

This program was built on Ubuntu Linux.  It is assumed the build system
will have standard developer tools like gcc, make, and perl, as well as
other common user tools like wget and ImageMagick convert.

IF all these tools are in place, "make" followed by "./test0.sh" should
produce an out.png similar to the example.png provided.

== Features

The clock shows local rise, set, and transit times for the moon and the
sun, as well as local twilight times.  It has also been tested for a
large number of anomolous cases that occur near the poles, where there
are days when the sun or moon neither rises nor sets.

== Drawing

The program does all its own drawing routines into a memory buffer.
The hopeful intent is that some day this may make it onto a small
processor on a watchlike embedded device.

