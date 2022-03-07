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

# TODO FIX ad hoc sloppy parser, replace w/json

#$location = "34.0007,-81.0348";
$location = "$ARGV[0],$ARGV[1]";

print "$location\n";

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
#print $line;
      $line =~ s/(https[^\"]*)/$furl=$1/ge;
#print "$furl\n";

      @step2 = `wget "$furl" -O -`;
      $number = -1;
      foreach $fline (@step2) {
         $fline =~ s/[\x0a\x0d]//g;
         if ($fline =~ /number/) {
            $number = $fline;
            $number =~ s/[^0-9]//g;
#print "number $number\n";
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
   $today_shortforecast = $w{"1"}{"shortForecast"};
   if (length($w{"1"}{"temperature"})) {
      $today_temp = $w{"1"}{"temperature"} . "°" . $w{"1"}{"temperatureUnit"};
   }
   if (length($w{"1"}{"windSpeed"})) {
      $today_wind = $w{"1"}{"windSpeed"} . " " .  $w{"1"}{"windDirection"};
   }
}

if (2) { # $w{"2"}{"name"} eq "Tonight") {
   $tonight_shortforecast = $w{"1"}{"shortForecast"};
   if (length($w{"1"}{"temperature"})) {
      $tonight_temp = $w{"2"}{"temperature"} . "°" . $w{"2"}{"temperatureUnit"};
   }
   if (length($w{"1"}{"windSpeed"})) {
      $tonight_wind = $w{"2"}{"windSpeed"} . " " .  $w{"2"}{"windDirection"};
   }
}

# this will need refinement
$day{"Clear"} = "clear.png";
$night{"Clear"} = "nt_clear.png";

$today_shortforecast =~ s/Sunny/Clear/g;
$tonight_shortforecast =~ s/Sunny/Clear/g;

sub lookup_png($) {
   my ($arg) = @_;
   my $ret;
   open FILE, "<fonts/icons_128x128.h";
   while (<FILE>) {
      s/[\x0a\x0d]//g;
      if (/\/\/ $arg/) {
         my $tmp = $_;
         $tmp =~ s/'(.)'/$char=$1/ge; # TODO FIX
         $ret = "$char $arg";
         last;
      }
   }
   close FILE;
   return $ret;
}

sub lookup_day_png($) {
   my ($arg) = @_;
   my $png = $day{$arg};
   lookup_png($png);
}

sub lookup_night_png($) {
   my ($arg) = @_;
   my $png = $night{$arg};
   lookup_png($png);
}

print "$today_shortforecast\n";
print lookup_day_png("$today_shortforecast") . "\n";
print "$today_temp\n";
print "$today_wind\n";
print "$tonight_shortforecast\n";
print lookup_night_png("$tonight_shortforecast") . "\n";
print "$tonight_temp\n";
print "$tonight_wind\n";
