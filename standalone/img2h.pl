#!/usr/bin/perl

$src = $ARGV[0];

@xpm = `convert \"$src\" -scale 360x180 -colors 96 xpm:-`;

while (!($xpm[0] =~ /^\"/)) {
   shift @xpm;
}

$fl = shift @xpm;
$fl =~ s/\"//g;
@fl = split / /, $fl;
($x, $y, $c, $n, $j) = @fl;

if ($n != 1) {
   die "too many chars per pixel!\n";
}

print "#define MOON_XPM_X $x\n";
print "#define MOON_XPM_Y $y\n";
print "#define MOON_XPM_C $c\n";

for ($i = 0; $i < $c; $i++) {
   $l = shift @xpm;
   # "  c #3C3D3C",
   $l =~ s/^.([ -~]) c #([^"]+)./$char = $1, $hex = $2/ge;
   $cn{$char} = $i;
   $ch{$i} = $hex;
}

print "static unsigned int moon_xpm_palette[] = {\n";
print "#ifndef STANDALONE\n";
for ($i = 0; $i < $c; $i++) {
   print "   0xFF$ch{$i},\n";
}
print "#else // STANDALONE\n";
for ($i = 0; $i < $c; $i++) {
   $hex = $ch{$i};
   $hex =~ s/(..)(..)(..)/$r = $1, $g = $2, $b = $3/ge;
   print "   0xFF$b$g$r,\n";
}
print "#endif // !STANDALONE\n";
print "};\n";

shift @xpm;

print "static unsigned char moon_xpm_pixels[180][360] = {\n";
while ($xpm[0] =~ /^\"/) {
   $l = shift @xpm;
   $l =~ s/[\x0a\x0d\"]//g;
   $l =~ s/,$//g;
   @l = split //, $l;

   print "   { ";
   foreach $l (@l) {
      print "$cn{$l}, ";
   }
   print "},\n";
}
print "};\n";
