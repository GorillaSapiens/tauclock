#!/usr/bin/env perl

use utf8;
binmode(STDOUT, ":utf8");

$load;
$n = 0;
$m = sprintf("primer_%03d.png", $n);
@crop = ();

sub popcrop() {
   if ($#crop < 0) {
      return;
   }
   my $xyxy = pop @crop;
   my $ifname = $m;
   my $ofname = $m;
   $ofname =~ s/\.png/_z.png/g;
   my ($x1, $y1, $x2, $y2) = split /,/, $xyxy;
   $x1 -= 50;
   $y1 -= 50;
   $x2 += 50;
   $y2 += 50;
   my $w = $x2 - $x1;
   my $h = $y2 - $y1;
   my $cmd = "convert img/$ifname -crop $w"."x$h+$x1+$y1 img/$ofname";
   print "$cmd\n";
   `$cmd`;
}

sub rectangle($$$$) {
   my ($x1,$y1,$x2,$y2) = @_;
   push @crop, "$x1,$y1,$x2,$y2";
   return "-draw \"rectangle $x1,$y1 $x2,$y2\" ";
}

sub roundrectangle($$$$) {
   my ($x1,$y1,$x2,$y2) = @_;
   return "-draw \"roundrectangle $x1,$y1 $x2,$y2 32,32\" ";
}

sub line($$$$) {
   my ($x1,$y1,$x2,$y2) = @_;
   return "-draw \"line $x1,$y1 $x2,$y2\" ";
}

sub caption($$$$$) {
   my ($x1,$y1,$x2,$y2,$t) = @_;
   my $w = $x2 - $x1;
   my $h = $y2 - $y1;
   return "-size $w" . "x$h! -gravity center caption:\"$t\" -gravity NorthWest -geometry +$x1+$y1 -composite ";
}

sub escape($) {
   my ($arg) = @_;
   $arg =~ s/\\/\\\\/g;
   $arg =~ s/\"/\\\"/g;
   return $arg;
}

sub captionboxpre() {
   return "-strokewidth 10 -stroke red -fill white ";
}

sub rectanglepre() {
   return "-strokewidth 10 -stroke red -fill transparent ";
}

sub captiontextpre() {
   return "-stroke transparent -fill black ";
}

sub text($$$$$) {
   my ($x1, $y1, $x2, $y2, $t) = @_;
   my $x1m = $x1 + 50;
   my $x2m = $x2 - 50;
   my $y1m = $y1 + 50;
   my $y2m = $y2 - 50;

   return captionboxpre() .
            roundrectangle($x1,$y1,$x2,$y2) .
            captiontextpre() .
            caption($x1m,$y1m,$x2m,$y2m,$t);
}

# primer intro
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   text(100,600,1300,900,"ταμ clock primer") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# weekday name
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(621,1460,818,1540) .
   text(100,900,1300,1200,"weekday name") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# month and date
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(627,1526,826,1577) .
   text(100,900,1300,1200,"month name and date") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# current time
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(592,1584,854,1672) .
   text(100,900,1300,1200,"current time") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# current location
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(565,1675,874,1724) .
   text(100,900,1300,1200,"current location") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# ISO-8601 date
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(640,1720,811,1763) .
   text(100,900,1300,1200,"ISO 8601 date") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# now hand
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(360,1200,624,1520) .
   text(100,600,1300,900,"white 'now hand' indicates current time") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# hour ticks
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(510,1110,600,1200) .
   text(100,600,1300,900,"tick marks every hour around edge") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# colors
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(900,1480,1060,1880) .
   text(100,600,1300,900,"colors indicate sun position") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# colors yellow
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(900,1480,1060,1880) .
   text(100,600,1300,900,"yellow = sun above horizon") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# colors orange
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(900,1480,1060,1880) .
   text(100,600,1300,900,"orange = civil twilight") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# colors light blue
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(900,1480,1060,1880) .
   text(100,600,1300,900,"light blue = nautical twilight") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# colors blue
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(900,1480,1060,1880) .
   text(100,600,1300,900,"blue = astronomical twilight") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# colors dark blue
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(900,1480,1060,1880) .
   text(100,600,1300,900,"dark blue = darkness") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# edge colors
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(1226,1646,1272,1826) .
   text(100,600,1300,900,"edge color indicates light/dark") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# edge colors white
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(1226,1646,1272,1826) .
   text(100,600,1300,900,"white = light") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# edge colors light gray
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(1226,1646,1272,1826) .
   text(100,600,1300,900,"light gray = twilight") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# edge colors dark gray
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(1226,1646,1272,1826) .
   text(100,600,1300,900,"dark gray = dark") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# top time
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(646,1162,792,1222) .
   text(100,600,1300,900,"top time indicates solar noon") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# top duration
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(622,1208,812,1328) .
   text(100,600,1300,900,"top duration shows length of light") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

# bottom duration
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(626,1934,822,2054) .
   text(100,600,1300,900,"bottom duration shows length of dark") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop();
$n++; $m = sprintf("primer_%03d.png", $n);

