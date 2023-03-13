#!/usr/bin/env perl

use utf8;
binmode(STDOUT, ":utf8");

$load;
$n = 0;
$m = sprintf("primer_%03d.png", $n);
@crop = ();

sub do_html() {
   my $nn = $n - 1;
   if ($nn < 0) {
      return;
   }
   my $xname = sprintf("img/primer_%03d.png", $nn + 1);

   my $fname = sprintf("primer_%03d.html", $nn);
   open TEMPLATE, "<template.html";
   open FILE, ">$fname";

   while (<TEMPLATE>) {
      if (/PREV/) {
         if ($nn == 0) {
            s/[ -~]/&nbsp;/g;
         }
         else {
            $prev = sprintf("primer_%03d.html", $nn - 1);
            s/ //g;
            $_ =~ s/^(.*)$/"<a href=$prev>$1<\/a>"/ge;
         }
      }
      if (/NEXT/) {
         if (-e $xname) {
            $next = sprintf("primer_%03d.html", $nn + 1);
            s/ //g;
            s/^(.*)$/"<a href=$next>$1<\/a>"/ge;
         }
         else {
            s/[ -~]/&nbsp;/g;
         }
      }
      $pn = sprintf("%03d", $nn);
      s/_XXX/"_".$pn/ge;
      print FILE $_;
   }
   close FILE;
   close TEMPLATE;
}

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
   my $scale = "";
   if ($w > 512) {
      $scale = "-scale 50%";
   }
   my $cmd = "convert img/$ifname -crop $w"."x$h+$x1+$y1 $scale img/$ofname";
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
push @crop, "100,600,1300,900";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# weekday name
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(621,1460,818,1540) .
   text(100,900,1300,1200,"weekday name") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# month and date
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(627,1526,826,1577) .
   text(100,900,1300,1200,"month name and date") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# current time
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(592,1584,854,1672) .
   text(100,900,1300,1200,"current time") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# current location
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(565,1675,874,1724) .
   text(100,900,1300,1200,"current location") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# ISO-8601 date
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(640,1720,811,1763) .
   text(100,900,1300,1200,"ISO 8601 date") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# now hand
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(360,1200,624,1520) .
   text(100,600,1300,900,"white 'now hand' indicates current time") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# hour ticks
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(510,1110,600,1200) .
   text(100,600,1300,900,"tick marks every hour around edge") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# colors
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(900,1480,1060,1880) .
   text(100,600,1300,1400,
      "colors indicate sun position:\\n".
      "yellow = sun up\\n".
      "orange = civil twilight\\n".
      "light blue = nautical twilight\\n".
      "blue = astronomical twilight\\n".
      "dark blue = night") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# edge colors
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(1226,1646,1272,1826) .
   text(100,600,1300,1500,"edge color indicates light/dark:\\n" .
      "white = light\\n" .
      "light gray = twilight\\n" .
      "dark gray = dark") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# top time
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(646,1162,792,1222) .
   text(100,600,1300,900,"top time indicates solar noon") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# top duration
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(622,1208,812,1328) .
   text(100,600,1300,900,"top duration shows length of light") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# bottom duration
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(626,1934,822,2054) .
   text(100,600,1300,900,"bottom duration shows length of dark") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# moon
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(420,1640,576,1796) .
   text(100,600,1300,900,"phase of the moon") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# moon down
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(420,1640,576,1796) .
   text(100,600,1300,900,"gray edge indicates moon is down") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# moon up
$load = "tauclock_0_26_main_b.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(425,1663,572,1799) .
   text(100,600,1300,900,"white edge indicates moon is up") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# sun rise times
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(208,1591,387,1820) .
   text(100,600,1300,900,"sun rise times\\nitalic text = in the past") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# sun set times
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(1050,1580,1216,1821) .
   text(100,600,1300,900,"sun set times\\nbold text = in the future") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# planets
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(0,1122,254,1844) .
   text(100,600,1300,900,"rise, transit, and set times for moon & planets") .
   "-font DejaVu-Sans-Mono " .
   text(300,930,1394,1900, "purple ☽︎ = Moon\\n" .
      "gray ☿ = Mercury\\n" .
      "white ♀ = Venus\\n" .
      "red ♂ = Mars\\n" .
      "orange ♃ = Jupiter\\n" .
      "lt blue ♄ = Saturn\\n" .
      "green ♈︎= Aries") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# julian date
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(1020,2286,1438,2374) .
   text(100,600,1300,900,"current Julian Date") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# location provider
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(0,900,240,1000) .
   text(212,1238,1322,2094,"location provider") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# location button
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(0,360,440,526) .
   text(212,1238,1322,2094,"location button\\n" .
      "cycles through location providers") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# timezone provider
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(0,2180,412,2400) .
   text(212,1238,1322,2094,"timezone provider\\n".
      "shows provider and current timezone") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# timezone button
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(0,2760,344,2906) .
   text(212,1238,1322,2094,"timezone button\\n" .
      "cycles through timezone providers") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# fwd/back button
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(348,2760,1020,2900) .
   text(212,1238,1322,2094,"forward and back buttons\\n" .
      "move clock forward or back when offset is set to manual") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# alarm button
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(1060,360,1430,520) .
   text(212,1238,1322,2094,"alarm button\\n" .
      "add / edit / delete alarms based on rise / transit / set events") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# settings button
$load = "tauclock_0_26_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(1020,2750,1420,2910) .
   text(212,1238,1322,2094,"settings button\\n" .
      "change application settings") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);

# light / darkness
$load = "tauclock_0_26_settings_main.png";
$cmd = "convert $load " .
   rectanglepre() .
   rectangle(170,380,1400,1200) .
   text(70,1290,1375,2600,"light / dark sliders\\n" .
      "lets you choose which sun positions count as light or dark") .
   "img/$m";
print "$cmd\n";
`$cmd`; popcrop(); do_html();
$n++; $m = sprintf("primer_%03d.png", $n);


####################### FINI
do_html(); # for the last page
