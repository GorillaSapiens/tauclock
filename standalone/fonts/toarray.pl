#!/usr/bin/env perl

$file = $ARGV[0];
$token = $ARGV[1];

$size = -s $file;

$cmd = "od -v -An -t x1 $file | sed \"s/ /, 0x/g\" | sed \"s/^, //g\" | sed \"s/\$/,/g\" | sed \"\$ s/,\$//g\"";

@cmd = `$cmd`;

print "unsigned char $token\[$size\] = {\n";
print @cmd;
print "};\n";
