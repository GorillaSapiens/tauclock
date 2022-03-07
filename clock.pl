#!/usr/bin/perl

##  Sunclock, draw a clock with local solar and lunar information
##  Copyright (C) 2022 Adam Wozniak / GorillaSapiens
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <https://www.gnu.org/licenses/>.

use Date::DayOfWeek;

@days = (
      "Sunday",
      "Monday",
      "Tuesday",
      "Wednesday",
      "Thursday",
      "Friday",
      "Saturday" );

@months = (
      "January",
      "February",
      "March",
      "April",
      "May",
      "June",
      "July",
      "August",
      "September",
      "October",
      "November",
      "December");

sub reg($$) {
   $time{$_[0]} = $_[1];
   ($h, $m, $s) = split /:/, $_[1];
   $angle{$_[0]} = ($h + $m/60 + $s/3600) / 24;
}

while (<>) {
   s/[\x0a\x0d]//g;
   my $tmp = $_;
   s/\[([^\]]+)\] [-0-9]* ([:0-9]*)/reg($1,$2)/ge;
   s/lunar ([a-z_]*) ([-0-9.]*)/$lunar{$1}=$2/ge;
   s/location ([-0-9.,]*)/$location=$1/ge;
   if ($tmp =~ /Now/) {
      $tmp =~ s/\[([^\]]+)\] ([-0-9]*) [:0-9]*/$today_date = $2/ge;
      my ($y, $m, $d) = split /-/, $today_date;
      my $dow = dayofweek( $d, $m, $y ); # dd, mm, yyyy
      $today_longdate = $days[$dow] . ", " . $months[$m-1] . " " . $d . ", " . $y;
   }
}

foreach $key(keys(%time)) {
   if ($key ne "Sun Transit") {
      $angle{$key} -= $angle{"Sun Transit"};
      if ($angle{$key} < 0) {
         $angle{$key} += 1;
      }
   }
}
$angle{"Sun Transit"} = 0;

foreach $key(keys(%time)) {
   $time{$key} =~ s/:[0-9][0-9]$//g;
   print "{$key} $time{$key} $angle{$key}\n";
}

foreach $key(keys(%lunar)) {
   print "{$key} $lunar{$key}\n";
}

$size = 1024;
$mid = $size / 2;
$cross="x";
$cmd = "convert -size $size$cross$size canvas:white ";

sub line($$$$) {
   my ($cx, $cy, $angle, $len) = @_;
   my $x = $cx + $len * sin(2.0 * 3.14159265359 * $angle);
   my $y = $cy - $len * cos(2.0 * 3.14159265359 * $angle);
   return (int($x), int($y));
}

sub arc($$) {
   my ($rise, $set) = @_;

   $rise *= 360;
   $set *= 360;

   $rise += 270;
   $set += 270;

   if ($rise > 360) {
      my $tmp = $rise;
      my $rise = $set;
      my $set = $tmp;
   }

   return (int($rise), int($set));
}

sub do_text($$$$$) {
   my ($x, $y, $t, $front, $back) = @_;
   my $ret = "";

   for (my $dx = -3; $dx < 4; $dx++) {
      for (my $dy = -3; $dy < 4; $dy++) {
         my $xx = $x + $dx;
         my $yy = $y + $dy;

         $ret .= "-draw \"fill $back stroke $back text $xx,$yy '$t'\" ";
      }
   }

   $ret .= "-draw \"fill $front stroke $front text $x,$y '$t'\" ";

   return $ret;
}

sub moonmagic($$$) {
   my ($a, $b, $c) = @_;

# fudge adjustment for perfect 50%

   if ($b == 0) {
      if ($c) {
         $b = 1;
      }
      else {
         $b = -1;
      }
   }

# from wolfram alpha
# a circle passing through points (0,a), (0,-a), and (b,0)

   my $center_x = ($b * $b - $a * $a) / (2 * $b);

   return int($center_x);
}

### DRAW THE MOON

($actual_moon_x,$actual_moon_y) =
        line($mid, $mid, $angle{"Moon Transit"}, $mid - 52);

$moon_dark = "black";
$moon_light ="gray";
$background = "white";
$moon_radius = 40;
$moon_img_size = 96;
$moon_x = $moon_y = int($moon_img_size / 2);

$phase = int(($lunar{"phase"} * 2 * $moon_radius / 180) - $moon_radius);
$moonmagic = moonmagic($moon_radius, $phase, $actual_moon_x <= $mid);

print "\n### $actual_moon_x $actual_moon_y $mid\n\n";
if ($actual_moon_x <= $mid) {
   if ($actual_moon_y <= $mid) {
      $moon_inner = $moon_light;
      $moon_chunk = $moon_dark;
      $chunk_x = $moon_x - $moonmagic;
      $chunk_x2 = $moon_x - $phase;
      $chunk_y = $moon_y;
   }
   else {
      $moon_inner = $moon_dark;
      $moon_chunk = $moon_light;
      $chunk_x = $moon_x - $moonmagic;
      $chunk_x2 = $moon_x - $phase;
      $chunk_y = $moon_y;
   }
}
else {
   if ($actual_moon_y <= $mid) {
      $moon_inner = $moon_light;
      $moon_chunk = $moon_dark;
      $chunk_x = $moon_x + $moonmagic;
      $chunk_x2 = $moon_x + $phase;
      $chunk_y = $moon_y;
   }
   else {
      $moon_inner = $moon_dark;
      $moon_chunk = $moon_light;
      $chunk_x = $moon_x + $moonmagic;
      $chunk_x2 = $moon_x + $phase;
      $chunk_y = $moon_y;
   }
}

$moon_x2 = $moon_x + 40;

$cmd2 = "convert -size $moon_img_size$cross$moon_img_size canvas:white ";

# draw the circle
$cmd2 .= "-strokewidth 0 -draw 'fill $moon_inner stroke $moon_inner circle $moon_x $moon_y $moon_x2 $moon_y' ";

# draw the chunk
$cmd2 .= "-strokewidth 0 -draw 'fill $moon_chunk stroke $moon_chunk circle $chunk_x $chunk_y $chunk_x2 $chunk_y' ";

# draw the ring
$cmd2 .= "-strokewidth 5 -draw 'fill none stroke blue circle $moon_x $moon_y $moon_x2 $moon_y' ";

# do the fill
$cmd2 .= "-fill $moon_chunk -fuzz 25% -draw 'color 1,1 floodfill' ";
$cmd2 .= "-fill $background -fuzz 25% -draw 'color 1,1 floodfill' ";

# do the ring again
$cmd2 .= "-strokewidth 5 -draw 'fill none stroke lightgray circle $moon_x $moon_y $moon_x2 $moon_y' ";

$cmd2 .= "moon.xpm";

print "$cmd2\n";
`$cmd2`;

$actual_moon_x -= int($moon_img_size / 2);
$actual_moon_y -= int($moon_img_size / 2);

$cmd .= "moon.xpm -geometry +$actual_moon_x+$actual_moon_y -composite ";

### DONE DRAW THE MOON

#$cmd .= "-strokewidth 1 -draw 'fill white stroke white circle $mid $mid $mid 136' ";

($a,$b) = arc($angle{"Moon Rise"}, $angle{"Moon Set"});
$x1 = $y1 = 128-16;
$x2 = $y2 = $size - 128 + 16;
$cmd .= "-strokewidth 15 -draw 'fill none stroke purple arc $x1,$y1 $x2,$y2 $a,$b' ";

($x_alpha,$y_alpha) = line($mid, $mid, $angle{"Moon Transit"}, $mid - 128);
($x_beta,$y_beta) = line($mid, $mid, $angle{"Moon Transit"}, $mid - 128 + 32);
$cmd .= "-strokewidth 5 -draw 'fill purple stroke purple line $x_alpha $y_alpha $x_beta $y_beta' ";

($x_alpha,$y_alpha) = line($mid, $mid, $angle{"Moon Rise"}, $mid - 128);
($x_beta,$y_beta) = line($mid, $mid, $angle{"Moon Rise"}, $mid - 128 + 32);
$cmd .= "-strokewidth 5 -draw 'fill purple stroke purple line $x_alpha $y_alpha $x_beta $y_beta' ";

($x_alpha,$y_alpha) = line($mid, $mid, $angle{"Moon Set"}, $mid - 128);
($x_beta,$y_beta) = line($mid, $mid, $angle{"Moon Set"}, $mid - 128 + 32);
$cmd .= "-strokewidth 5 -draw 'fill purple stroke purple line $x_alpha $y_alpha $x_beta $y_beta' ";

$xx1 = $x1 + 21;
$yy1 = $y1 + 21;
$xx2 = $x2 - 21;
$yy2 = $y2 - 21;
$cmd .= "-strokewidth 3 -draw 'fill none stroke black arc $xx1,$yy1 $xx2,$yy2 0,360' ";

$xx1 = $x1 + 217;
$yy1 = $y1 + 217;
$xx2 = $x2 - 217;
$yy2 = $y2 - 217;
$cmd .= "-strokewidth 3 -draw 'fill none stroke black arc $xx1,$yy1 $xx2,$yy2 0,360' ";

$thingy = 96;

$x1 += 23 + $thingy;
$y1 += 23 + $thingy;
$x2 -= 23 + $thingy;
$y2 -= 23 + $thingy;

$thingy *= 2;
$cmd .= "-strokewidth $thingy ";

$cmd .= "-draw 'fill none stroke darkblue arc $x1,$y1 $x2,$y2 0,360' ";

if (defined($angle{"Astronomical Rise"})) {
   ($a,$b) = arc($angle{"Astronomical Rise"}, $angle{"Astronomical Set"});
   $cmd .= "-draw 'fill none stroke blue arc $x1,$y1 $x2,$y2 $a,$b' ";
}

if (defined($angle{"Nautical Rise"})) {
   ($a,$b) = arc($angle{"Nautical Rise"}, $angle{"Nautical Set"});
   $cmd .= "-draw 'fill none stroke #4cabf4 arc $x1,$y1 $x2,$y2 $a,$b' ";
}

if (defined($angle{"Civil Rise"})) {
   ($a,$b) = arc($angle{"Civil Rise"}, $angle{"Civil Set"});
   $cmd .= "-draw 'fill none stroke orange arc $x1,$y1 $x2,$y2 $a,$b' ";
}

if (defined($angle{"Sun Rise"})) {
   ($a,$b) = arc($angle{"Sun Rise"}, $angle{"Sun Set"});
   $cmd .= "-draw 'fill none stroke yellow arc $x1,$y1 $x2,$y2 $a,$b' ";
}

####

$cmd .= "-strokewidth 2 ";

# HOUR MARKS
$r = 256 + 128 - 2;
($h, $m) = split /:/, $time{"Sun Transit"};
$offset = 2 * 3.14159265359 * $m / 60 / 24;
for ($i = 0; $i < 24; $i++) {
   $x = $mid + $r * cos(2 * 3.14159265359 * $i / 24 - $offset);
   $y = $mid + $r * sin(2 * 3.14159265359 * $i / 24 - $offset);
   $y2 = $y + 5;
   $cmd .= "-draw 'fill black stroke white circle $x,$y $x,$y2' ";
}

####

## WEATHER

# TODO FIX ad hoc sloppy parser, replace w/json

sub remember($$$) {
   my ($n, $k, $v) = @_;
   $v =~ s/,$//g;
   $v =~ s/^\"//g;
   $v =~ s/\"$//g;
   $w{$n}{$k} = $v;
}

@step1 = `wget "https://api.weather.gov/points/$location" -O -`;
foreach $line (@step1) {
   if ($line =~ /\"forecast\"/) {
      print $line;
      $line =~ s/(https[^\"]*)/$furl=$1/ge;
      print "$furl\n";

      @step2 = `wget "$furl" -O -`;
      $number = -1;
      foreach $fline (@step2) {
         $fline =~ s/[\x0a\x0d]//g;
         if ($fline =~ /number/) {
            $number = $fline;
            $number =~ s/[^0-9]//g;
            print "number $number\n";
         }

         if ($number != -1) {
            if ($fline =~ /:/) {
               $fline =~ s/\"([a-zA-Z0-9_]*)\": (.*)$/remember($number,$1,$2)/ge;
            }
         }
      }
   }
}

if (1) { # $w{"1"}{"name"} eq "Today") {
   $icon = $w{"1"}{"shortForecast"};

   $today_short = $icon;
   if ($today_short eq "Sunny") {
      $today_short = "Clear";
   }

   $icon = lc($icon);
   $icon =~ s/ //g;
   $icon =~ s/and.*//g;
   print "TODAY ICON $icon\n";
   $ix = $mid - 128/2;
   $iy = $mid / 2 - 128/2;
   if (length($icon)) {
      $cmd .= "icons/solid-black/png/128x128/$icon.png -geometry +$ix+$iy -composite ";
   }
   if (length($w{"1"}{"temperature"})) {
      $today_temp = $w{"1"}{"temperature"} . "°" . $w{"1"}{"temperatureUnit"};
   }
   if (length($w{"1"}{"windSpeed"})) {
      $today_wind = $w{"1"}{"windSpeed"} . " " .  $w{"1"}{"windDirection"};
   }
}

if (2) { # $w{"2"}{"name"} eq "Tonight") {
   $icon = $w{"1"}{"shortForecast"};

   $tonight_short = $icon;
   if ($tonight_short eq "Sunny") {
      $tonight_short = "Clear";
   }

   $icon = lc($icon);
   $icon =~ s/ //g;
   $icon =~ s/and.*//g;
   print "TONIGHT ICON $icon\n";
   $ix = $mid - 128/2;
   $iy = $mid + $mid / 2 - 128/2;
   if (length($icon)) {
      $cmd .= "icons/solid-white/png/128x128/nt_$icon.png -geometry +$ix+$iy -composite ";
   }
   if (length($w{"1"}{"temperature"})) {
      $tonight_temp = $w{"2"}{"temperature"} . "°" . $w{"2"}{"temperatureUnit"};
   }
   if (length($w{"1"}{"windSpeed"})) {
      $tonight_wind = $w{"2"}{"windSpeed"} . " " .  $w{"2"}{"windDirection"};
   }
}

####

($x,$y) = line($mid, $mid, $angle{"Now"}, $mid - 128);
$cmd .= "-strokewidth 5 -draw 'fill gray stroke gray line $mid $mid $x $y' ";

$cmd .= "-gravity center -strokewidth 1 -pointsize 128 ";

($x,$y) = ($mid, $mid);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Now"}, "black", "white");

$cmd .= "-pointsize 24 ";

$y += 32;
$cmd .= do_text($x, $y, $today_longdate, "black", "white");

$y += 32;
$cmd .= do_text($x, $y, $today_date, "black", "white");

$y += 32;
($lat,$lon) = split /,/, $location;
if ($lat < 0) {
   $lat = -$lat;
   $lat_a = "S";
}
else {
   $lat_a = "N";
}
if ($lon < 0) {
   $lon = -$lon;
   $lon_a = "W";
}
else {
   $lon_a = "E";
}
$location_print = sprintf("%1.4f%s,%1.4f%s", $lat, $lat_a, $lon, $lon_a);
$cmd .= do_text($x, $y, $location_print, "black", "white");

($x,$y) = line($mid, $mid, $angle{"Sun Rise"}, $mid * 5 / 8);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Sun Rise"}, "black", "yellow");

($x,$y) = line($mid, $mid, $angle{"Sun Set"}, $mid * 5 / 8);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Sun Set"}, "black", "yellow");

($x,$y) = line($mid, $mid, $angle{"Sun Transit"}, $mid * 5 / 8);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Sun Transit"}, "black", "yellow");



($x,$y) = line($mid, $mid, $angle{"Astronomical Rise"}, $mid * 5 / 8);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Astronomical Rise"}, "white", "blue");

($x,$y) = line($mid, $mid, $angle{"Astronomical Set"}, $mid * 5 / 8);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Astronomical Set"}, "white", "blue");


($x,$y) = line($mid, $mid, $angle{"Nautical Rise"}, $mid * 5 / 8);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Nautical Rise"}, "black", "white"); #"#4cabf4");

($x,$y) = line($mid, $mid, $angle{"Nautical Set"}, $mid * 5 / 8);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Nautical Set"}, "black", "white"); #"#4cabf4");


($x,$y) = line($mid, $mid, $angle{"Civil Rise"}, $mid * 5 / 8);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Civil Rise"}, "black", "orange");

($x,$y) = line($mid, $mid, $angle{"Civil Set"}, $mid * 5 / 8);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Civil Set"}, "black", "orange");


($x,$y) = line($mid, $mid, $angle{"Moon Rise"}, $mid - 128 + 48);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Moon Rise"}, "black", "white");

($x,$y) = line($mid, $mid, $angle{"Moon Set"}, $mid - 128 + 48);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Moon Set"}, "black", "white");

($x,$y) = line($mid, $mid, $angle{"Moon Transit"}, $mid - 128 + 48);
$x -= $mid; $y -= $mid;
$cmd .= do_text($x, $y, $time{"Moon Transit"}, "black", "white");

if (length($today_temp)) {
   ($x,$y) = ($mid, $mid);
   $x -= $mid; $y -= $mid;
   $y -= 256 + 8; $x += 128 - 8;
   $cmd .= do_text($x, $y, $today_temp, "black", "yellow");
   $y += 32;
   $cmd .= do_text($x, $y, $today_wind, "black", "yellow");
   $y -= 16;
   $x -= 120 * 2;
   $cmd .= do_text($x, $y, $today_short, "black", "yellow");
}

if (length($tonight_temp)) {
   ($x,$y) = ($mid, $mid);
   $x -= $mid; $y -= $mid;
   $y += 256 - 8; $x += 128 - 8;
   $cmd .= do_text($x, $y, $tonight_temp, "white", "darkblue");
   $y += 32;
   $cmd .= do_text($x, $y, $tonight_wind, "white", "darkblue");
   $y -= 16;
   $x -= 120 * 2;
   $cmd .= do_text($x, $y, $tonight_short, "white", "darkblue");
}

$cmd .= "out.png";

print "$cmd\n";
`$cmd`;
