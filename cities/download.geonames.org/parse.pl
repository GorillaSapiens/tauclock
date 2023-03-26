#!/usr/bin/perl

# wget http://download.geonames.org/export/dump/allCountries.zip

$init = 0;
while (<>) {
	s/[\x0a\x0d]//g;

	if (!$init) {
		@fields = split /;/, $_;
		$init = 1;
	}
	else {
		@values = split /;/, $_;
		for ($i = 0; $i <= $#fields; $i++) {
			$thing{$fields[$i]} = $values[$i];
		}
		
		@names = ();
		push @names, $thing{"Name"};
		push @names, $thing{"ASCII Name"};
		push @names, split /,/, $thing{"Alternate Names"};

		$admin1 = $thing{"Admin1 Code"};
		$country = $thing{"Country name EN"};
		$where = $thing{"Coordinates"};

		($lat,$lon) = split /,/, $where;
		$where = sprintf("%0.4f,%0.4f", $lat, $lon);

		foreach $name (@names) {
			$name =~ s/\t/ /g;
			$name =~ s/  / /g;
			$name =~ s/  / /g;
			$name =~ s/  / /g;
			$name =~ s/  / /g;
			$name =~ s/  / /g;
			$name =~ s/  / /g;
			$name =~ s/^ //g;
			$name =~ s/ $//g;
			if (length($name)) {
				if ($admin1 =~ /[A-Za-z]/) {
					$out = "$name,$admin1,$country,$where";
				}
				else {
					$out = "$name,$country,$where";
				}
				$out =~ s/  / /g;
				$out =~ s/  / /g;
				$out =~ s/  / /g;
				$out =~ s/  / /g;
				$out =~ s/  / /g;
				$out =~ s/  / /g;
				$out =~ s/, /,/g;
				$out =~ s/ ,/,/g;
				$out =~ s/\&/and/g;
				$out =~ s/[<>'"]//g;
				print "\t<item>$out</item>\n";
			}
		}
	}
}
