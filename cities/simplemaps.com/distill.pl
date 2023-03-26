#!/usr/bin/perl

# see https://stackoverflow.com/questions/24086660/perl-parsing-csv-file-with-embedded-commas

print "<string-array name=\"cities\">\n";

my $re = qr/(?| "\( ( [^()""]* ) \)" |  \( ( [^()]* ) \) |  " ( [^"]* ) " |  ( [^,]* ) ) , \s* /x;

$n = 0;
while (<>) {
	if ($n != 0) {
		@l = "$_," =~ /$re/g;
		($city,
		 $city_ascii,
		 $lat,
		 $lon,
		 $country,
		 $iso2,
		 $iso3,
		 $admin_name,
		 $capital,
		 $population,
		 $id) = @l;

	 	if ($admin_name =~ /, /) {
			($a, $b) = split /, /, $admin_name;
			$admin_name = "$b $a";
		}

		$city .= "," . $admin_name . "," . $iso3;
		$city =~ s/  / /g;
		$city =~ s/ $//g;
		$city =~ s/,,/,/g;
		$city =~ s/,$//g;

		$city_ascii .= "," . $admin_name . "," . $iso3;
		$city_ascii =~ s/  / /g;
		$city_ascii =~ s/ $//g;
		$city_ascii =~ s/,,/,/g;
		$city_ascii =~ s/,$//g;

		print "<item>$city,$lat,$lon</item>\n";
		if ($city_ascii ne $city) {
			print "<item>$city_ascii,$lat,$lon\</item>\n";
		}
	}
	$n++;
}

print "</string-array>\n";
