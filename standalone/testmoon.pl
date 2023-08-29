#!/usr/bin/perl

@x = (
"https://www.timeanddate.com/astronomy/australia/adelaide -34.9285 138.6007",
"https://www.timeanddate.com/astronomy/italy/rome 41.9028 12.4964",
"https://www.timeanddate.com/astronomy/antarctica/mcmurdo -77.8500 166.6667",
"https://www.timeanddate.com/astronomy/usa/honolulu 21.3099 -157.8581",
"https://www.timeanddate.com/astronomy/canada/tuktoyaktuk 69.4454 -133.0342",
);

foreach $x (@x) {
   ($url, $lat, $lon) = split / /, $x;
   $cmd = "(wget $url -O - 2>&1 | grep moon.php | sed \"s/.*moon.php//g\" | sed \"s/\\\".*//g\")";
   $cmd2 = "(./calcdata $lat $lon 2>&1 | grep rot=)";
   @a = `$cmd`;
   $a = $a[0];
   $a =~ s/[\x0a\x0d]//g;
   $a =~ s/.*r=//g;
   $a = $a * -180 / 3.1415926;

   @b = `$cmd2`;
   $b = $b[0];
   $b =~ s/[\x0a\x0d]//g;
   $b =~ s/.*rot=//g;
   print "url=$a\n";
   print "me =$b\n";
   print "\n";
}
