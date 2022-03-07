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

$dirname = $ARGV[0];
$dirname =~ s/\/$//g;

$varname = $dirname;
$varname =~ s/.*\///g;
$varname =~ s/[^A-Za-z0-9_]/_/g;
$varname = "icons_$varname";

$dimension = $varname;
$dimension =~ s/.*x//g;

open OUT, ">$varname.h";

print OUT "// file converted from $dirname\n";
print OUT "#include <stdint.h>\n";
print OUT "#ifndef INCLUDE_FONT_DATA\n";
print OUT "extern uint8_t $varname"."[];\n";
print OUT "#else\n";
print OUT "uint8_t $varname"."[] = {\n";
print OUT "   $dimension, $dimension, 0, 0, // width, height, dx, dy\n";

$encoding = 65;

foreach $file (`ls $dirname`) {
   $file =~ s/[\x0a\x0d]//g;
   if (!($file =~ /^_/)) {
      $hi = int($encoding / 256);
      $lo = $encoding % 256;

      if ($hi == 0 && $lo <= ord('~')) {
         if ($lo == ord("\\") || $lo == ord("\'")) {
            $lo = "'\\" . chr($lo) . "'";
         }
         else {
            $lo = "'" . chr($lo) . "'";
         }
      }

      print OUT "   $hi, $lo, // $file\n";
      print OUT "   $dimension, $dimension, 0, 0, // width, height, dx, dy\n";
      $encoding++;

      @cont = `convert $dirname/$file xpm:-`;

      foreach $line (@cont) {
         if ($line =~ / c None/) {
            $tmp = $line;
            $tmp =~ s/(.) c None/$none=$1/ge;
         }
      }

      $mode = 0;
      foreach $line (@cont) {
         $line =~ s/[\x0a\x0d]//g;
         if ($line =~ /pixels/) {
            $mode = 1;
         }
         elsif ($mode == 1) {
            if ($line =~ /^}/) {
               $mode = 0;
               print OUT "\n";
            }
            else {
               $line =~ s/^"//g;
               $line =~ s/".*//g;

               $hex = 0;
               $count = 0;
               @bits = split //, $line;
               while ($#bits != -1) {
                  $bit = shift @bits;
                  $hex <<= 1;
                  if ($bit ne $none) {
                     $hex |= 1;
                  }
                  $count++;
                  if ($count == 8) {
                     $count = 0;
                     $stuff = sprintf(" 0x%02x,", $hex);
                     print OUT $stuff;
                     $hex = 0;
                  }
               }
            }
         }
      }
   }
}

print OUT "   255, 255 }; // terminator\n"; # TODO FIX
print OUT "#endif\n";
