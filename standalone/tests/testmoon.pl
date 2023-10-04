#!/usr/bin/perl

sub normalize($) {
   my $arg = shift @_;
   while ($arg < 0) {
      $arg += 360.0;
   }
   while ($arg >= 360.0) {
      $arg -= 360.0;
   }
   return $arg;
}

@x = (
"https://www.timeanddate.com/astronomy/australia/adelaide -34.9285 138.6007",
"https://www.timeanddate.com/astronomy/italy/rome 41.9028 12.4964",
"https://www.timeanddate.com/astronomy/antarctica/mcmurdo -77.8500 166.6667",
"https://www.timeanddate.com/astronomy/usa/honolulu 21.3099 -157.8581",
"https://www.timeanddate.com/astronomy/canada/tuktoyaktuk 69.4454 -133.0342",
);

foreach $x (@x) {
   print "$x\n";
   ($url, $lat, $lon) = split / /, $x;
   $cmd = "(wget $url -O - 2>&1 | grep -a moon.php | sed \"s/.*moon.php//g\" | sed \"s/\\\".*//g\")";
   $cmd2 = "(./calcdata $lat $lon 2>&1 | grep -a rot=)";
   @a = `$cmd`;
   $a = $a[0];
   $a =~ s/[\x0a\x0d]//g;

   $ta = $a;
   $ta =~ s/i=([-0-9.]*)/$i = $1/ge;
   $ta = $a;
   $ta =~ s/p=([-0-9.]*)/$p = $1/ge;
   $ta = $a;
   $ta =~ s/r=([-0-9.]*)/$r = $1/ge;

   $i = $i;
   $p = normalize($p * 180 / 3.141592653587);
   $r = normalize($r * 180 / 3.141592653587);

   $pmr = normalize($p - $r);

   @b = `$cmd2`;
   $b = $b[0];
   $b =~ s/[\x0a\x0d]//g;
   $b =~ s/.*rot=//g;

   @b = split / /,$b;
   ($mi, $mbla, $mrot) = @b;
   $mbla = normalize($mbla);
   $mrot = normalize($mrot);

   print "url = i=$i (p=$p) r=$r (p-r)=$pmr\n";
   print "me  = i=$mi bla=$mbla rot=$mrot\n";
   print "\n";
}
