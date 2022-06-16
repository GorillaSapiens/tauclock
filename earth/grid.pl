#!/usr/bin/perl

$cmd  = "convert -size 2560x1280 xc:transparent";
$cmd .= " -fill CornflowerBlue -stroke CornflowerBlue -strokewidth 3";

for ($i = 0; $i < 36; $i++) {
$x = int(2560 * $i / 36);
$cmd .= " -draw \"line $x,0 $x,1279\"";
}

for ($i = 0; $i < 18; $i++) {
$y = int(1280 * $i / 18);
$cmd .= " -draw \"line 0,$y 2559,$y\"";
}

# tropic = equator +/- 23.5 degrees
# circle = pole +/- 23.5 degrees

$offset = int(1280 * 23.5 / 180);

$cmd .= " -fill LightSteelBlue -stroke LightSteelBlue -strokewidth 3";
$y = $offset;
$cmd .= " -draw \"line 0,$y 2559,$y\"";
$y = 1280 / 2 + $offset;
$cmd .= " -draw \"line 0,$y 2559,$y\"";
$y = 1280 / 2 - $offset;
$cmd .= " -draw \"line 0,$y 2559,$y\"";
$y = 1280 - $offset;
$cmd .= " -draw \"line 0,$y 2559,$y\"";

$cmd .= " -fill LemonChiffon -stroke LemonChiffon -strokewidth 3";
$cmd .= " -draw \"line 1280,0 1280,1279\"";
$cmd .= " -draw \"line 0,640 2559,640\"";
$cmd .= " grid.png";

print `$cmd`;
