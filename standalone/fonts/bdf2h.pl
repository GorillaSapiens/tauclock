#!/usr/bin/perl

# Sunclock, draw a clock with local solar and lunar information
# Copyright (C) 2022,2023 Adam Wozniak / GorillaSapiens
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

$filename = $ARGV[0];
print "$filename\n";

$varname = $filename;
$varname =~ s/.*\///g;
$varname =~ s/[^A-Za-z0-9_]/_/g;

open OUT, ">$varname.h";

open FONT, "<$filename" || die "unable to open $filename\n";

print OUT "// file converted from $filename\n";
print OUT "#include <stdint.h>\n";
print OUT "#ifndef INCLUDE_FONT_DATA\n";
print OUT "extern uint8_t $varname" . "[];\n";
print OUT "#else\n";
print OUT "uint8_t $varname"."[] = {\n";

$iscolon = 0;

$mode = 0;
while (<FONT>) {
   s/[\x0a\x0d]//g;
   if (/^FONTBOUNDINGBOX/) {
      s/^FONTBOUNDINGBOX[ ]+([0-9]+)[ ]+([0-9]+)[ ]+([-0-9]+)[ ]+([-0-9]+)/$width=$1,$height=$2,$deltax=$3,$deltay=$4/ge;
      print OUT "   $width, $height, $deltax, $deltay, // width, height, deltax, deltay\n";
      print OUT "\n";
      $step = int(($width+7)/8);
   }
   if (/^BBX/) {
      s/^BBX[ ]+([0-9]+)[ ]+([0-9]+)[ ]+([-0-9]+)[ ]+([-0-9]+)/$bbx_width=$1,$bbx_height=$2,$bbx_deltax=$3,$bbx_deltay=$4/ge;
      $bbx_step = int(($bbx_width + 7) / 8);
   }
   if (/^STARTCHAR/) {
      $comment = $_;
      $comment =~ s/^STARTCHAR //g;
      $bytenum = 0;
      $bbx_width = $width;
      $bbx_height = $height;
      $bbx_deltax = $deltax;
      $bbx_deltay = $deltay;
      $bbx_step = $step;
   }
   if (/^ENCODING/) {
      s/^ENCODING ([0-9]+)/$enc=$1/ge;
      if ($enc >= ord(' ') && $enc <= ord('~')) {
         if ($enc == ord('\\') || $enc == ord('\'')) {
            print OUT "   0x00, '\\".chr($enc)."', // $comment\n";
         }
         else {
            print OUT "   0x00, '".chr($enc)."', // $comment\n";
         }
      }
      else {
         $hi = int($enc / 256);
         $lo = $enc % 256;
         print OUT "   $hi, $lo, // $enc :: $comment\n";
      }
      if ($enc == ord(':')) {
         $iscolon = 1;
      }
      else {
         $iscolon = 0;
      }
   }
   if (/^BITMAP/) {
      $mode = 1;
      if ($iscolon) {
         $bbx_deltay = int(($height/2 - $bbx_height) / 2);
      }
      print OUT "   $bbx_width, $bbx_height, $bbx_deltax, $bbx_deltay, // bbx w,h,dx,dy\n";
      print OUT "  ";
   }
   elsif ($mode == 1) {
      if (/^ENDCHAR/) {
         $mode = 0;
         print OUT "\n";
      }
      else {
         my @output = ( m/.{2}/g );
         foreach $out (@output) {
            push @glyph, $out;
            print OUT " 0x$out,";
            $bytenum++;
            if ($bytenum % $bbx_step == 0) {
               print OUT "  // ";
               $bits = 0;
               while ($#glyph != -1) {
                  $hex = shift @glyph;
                  $hex = hex("0x".$hex);
                  $count = 8;
                  while ($count) {
                     if ($bits < $bbx_width) {
                        if ($hex & 0x80) {
                           print OUT "*";
                        }
                        else {
                           print OUT ".";
                        }
                        $hex <<= 1;
                     }
                     $bits++;
                     $count--;
                  }
               }
               print OUT "\n  ";
               @glyph = ();
            }
         }
      }
   }
}


print OUT "   255, 255, // 65535 :: terminator\n";
print OUT "   $width, $height, $deltax, $deltay, // bbx w,h,dx,dy\n";
print OUT "  ";
for ($j = 0; $j < $step; $j++) {
   print OUT " 0xff,";
}
print OUT "\n";
for ($i = 1; $i < $height - 1; $i++) {
   print OUT "  ";
   for ($j = 0; $j < $step; $j++) {
      if ($i % 2) {
         print OUT " 0xAA,";
      }
      else {
         print OUT " 0x55,";
      }
   }
   print OUT "\n";
}
print OUT "  ";
for ($j = 0; $j < $step; $j++) {
   print OUT " 0xff,";
}
print OUT "\n";
print OUT "\n";
print OUT "   0 };\n";
print OUT "#endif\n";

close FONT;
