#!/usr/bin/env perl

`rm out*`;
for ($i = 0; $i < 60*12; $i ++) {
   $j = 20230505.75+ $i/(24.0*60.0);
#$j += 365.0;
   print "$i\n";
   print `./calcdataprecise 19 73 $j`;
   $x = sprintf("out_%04d.png", $i);
   `convert -size 1024x1024 -depth 8 RGBA:out.bin $x`;
}

`convert -delay 25 out*.png out.gif`;
