#!/usr/bin/perl

$oii = "000";

for ($i = 0; $i < 360; $i++) {
   $j = $i - 180;
   print "$j\n";
   $ii = sprintf("%03d", $i);
   `../a.out 0 $j 0 > $ii.raw`;
   $oii = $ii;
}

for ($i = 0; $i < 360; $i++) {
   $j = $i - 180;
   print "$j\n";

   $a = sprintf("%03d", ($i - 1 + 360) % 360);
   $b = sprintf("%03d", $i % 360);
   $c = sprintf("%03d", ($i + 1) % 360);

#   `convert -size 256x256 -depth 8 RGB:$a.raw RGB:$b.raw RGB:$c.raw -evaluate-sequence mean $b.png`;
   `convert -size 256x256 -depth 8 RGB:$b.raw $b.png`;
}

`convert -delay 5 -loop 0 *.png spin.gif`;
