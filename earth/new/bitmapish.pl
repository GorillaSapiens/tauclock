#!/usr/bin/perl

@cont = <>;

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

# parse parameters
$params = $cont[$marker1 + 1];
$params =~ s/[\x0a\x0d]//g;
$params =~ s/[\"\,]//g;
($columns, $rows, $colors, $chars) = split / /, $params;

print "columns = $columns;\n";
print "rows = $rows;\n";
print "colors = $colors;\n";
print "chars = $chars;\n";

print "int palette[$colors] = {\n";
for ($i = 0; $i < $colors; $i++) {
   $line = $cont[$marker1 + 2 + $i];
   $line =~ s/[\x0a\x0d]//g;
   $line =~ s/[\",]//g;
   if (!($line =~ / c \#/)) {
      die "BAD COLOR $line\n";
   }
   $line =~ s/^.* c \#//g;
   print " 0x$line";
   if ($i != $colors - 1) {
      print ",";
   }
   if ($i % 8 == 7) {
      print "\n";
   }
}
if (($colors - 1) % 8 != 7) {
   print "\n";
}
print "};\n";
