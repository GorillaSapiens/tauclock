Sunclock

a clock for camping in the desert

== Compiling

This program was built on Ubuntu Linux.  It is assumed the build system
will have standard developer tools like gcc, make, and perl, as well as
other common user tools like wget, ffmpeg, and ImageMagick convert.

IF all these tools are in place, "make" followed by "./test0.sh" should
produce an out.png similar to the example.png provided.

== Features

The clock shows local rise, set, and transit times for the moon and the
sun, as well as local twilight times.  It has also been tested for a
large number of anomolous cases that occur near the poles, where there
are days when the sun or moon neither rises nor sets.

== Drawing

The program does all its own drawing routines into a memory buffer.
This is dumped out to a raw file, and ImageMagick convert can be used
to produce a png, or whatever format file you need.  The hopeful intent
is that some day this may make it onto a small processor on a watchlike
embedded device.

== Unimplemented Features

There is a weather.pl to fetch the weather, and code hooks to use and
display that information on the clock, but they are not used at this time.
Some early testers had trouble understanding some of the weather icons
(why does it show a crescent moon when the moon is full?) and I'm not
convinced an embedded watch device is ever going to have connectivity
to get this information.

There are hooks to draw Zodiac signs.  They are disabled, as it opened
the door for woo-woo astrology nuts who knew nothing about astronomy
to criticise the information being shown, and it was not generally
considered a useful addition for the intended use case of "someone
camping in the desert".

== Future features

Rise/set times of the other planets are possible.

One user asked for rise/set/transit times for the ISS.  While this is
possible, there are thousands of potentially interesting smaller bodies.
Also, the ISS, and presumably other similar objects, make frequent
course corrections, which require updating their Ephemeris data on a
monthly basis.  This was deemed "out of scope" for the intended use case.

Alarms set for "an hour before sunset" or "an hour after sunrise" are
possible, but were considered "out of scope" at this time.

== Fini

I hope you find this useful.  The home repository is
https://github.com/GorillaSapiens/sunclock
