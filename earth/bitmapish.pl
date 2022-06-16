#!/usr/bin/perl

#=================
# read input

@cont = <>;

#=================
# find markers

$state = 0;
for ($i = 0; $i <= $#cont; $i++) {
   if ($state == 0) {
      if ($cont[$i] =~ /columns rows colors chars-per-pixel/) {
         $state = 1;
         $marker1 = $i;
      }
   }
   elsif ($state == 1) {
      if ($cont[$i] =~ /pixels/) {
         $state = 2;
         $marker2 = $i;
         last;
      }
   }
}

#=================
# parse parameters

$params = $cont[$marker1 + 1];
$params =~ s/[\x0a\x0d,]*$//g;
$params =~ s/[\"]//g;
($columns, $rows, $colors, $chars) = split / /, $params;

print "int columns = $columns;\n";
print "int rows = $rows;\n";
print "int colors = $colors;\n";
print "//int chars = $chars;\n";

#=================
# process /usr/share/X11/rgb.txt

open FILE, "/usr/share/X11/rgb.txt";
while (<FILE>) {
   s/[\x0a\x0d]//g;
   if (!(/^\!/)) {
      s/\t/ /g;
      s/[ ]+/ /g;
      s/^[ ]+//g;
      @all = split / /;
      $r = shift @all;
      $g = shift @all;
      $b = shift @all;
      $name = join(" ", @all);
      $v = ($r << 16) | ($g << 8) | $b;
      $v = sprintf("%06x", $v);
      $rgb_txt{$name} = $v;
   }
}
close FILE;

#=================
# print palette

print "int palette[$colors] = {\n  ";
for ($i = 0; $i < $colors; $i++) {
   $line = $cont[$marker1 + 2 + $i];
   $line =~ s/[\x0a\x0d,]*$//g;
   $line =~ s/[\"]//g;

   if (!($line =~ / c \#/)) {
      $name = $line;
      $name =~ s/^.* c //g;
      $v = $rgb_txt{$name};
      $line =~ s/ c $name/ c #$v/g;
   }

   @line = split //, $line;
   while ($#line + 1 != $chars) {
      pop @line;
   }
   $sygil = join "", @line;
   $sygil{$sygil} = $i;

   $line =~ s/^.* c \#//g;
   print " 0x$line";
   if ($i != $colors - 1) {
      print ",";
   }
   if ($i % 4 == 3) {
      print "\n";
   }
   if ($i != $colors - 1) {
      print "  ";
   }
}
if (($colors - 1) % 4 != 3) {
   print "\n";
}
print "};\n";

#=================

print "unsigned char bitmap[$rows][$columns] = {\n";

for ($y = 0; $y < $rows; $y++) {
   $x = 0;
   print "   { ";

   $n = $marker2 + 1 + $y;
   $line = $cont[$n];
   $line =~ s/[\x0a\x0d]//g;
   $line =~ s/^"//g;
   $line =~ s/".*$//g;

   @line = split //, $line;
   $sygil = "";
   while ($#line >= 0) {
      while (length($sygil) != $chars) {
         $sygil .= shift(@line);
      }
      $c = $sygil{$sygil};
      printf(" 0x%02x", $c);
      $x++;
      if ($#line >= 0) {
         print ",";
         if ($x % 8 == 0) {
            print "\n     ";
         }
      }

      $sygil = "";
   }

   print "  }";
   if ($y != $rows - 1) {
      print ",";
   }
   print "\n";
}

print "};\n",
