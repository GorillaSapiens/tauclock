#!/usr/bin/perl

$oii = "000";

for ($i = 0; $i < 360; $i++) {
   $j = 360 - ($i - 180);
   print "$j\n";
   $ii = sprintf("%03d", $i);
   print `time ./a.out 512 0 $j 0 > /tmp/spin_$ii.raw`;
   $oii = $ii;
}

`convert -delay 5 -loop 0 -size 512x512 -depth 8 RGB:/tmp/spin_*.raw spin.gif`;
