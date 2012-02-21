#!/usr/bin/perl -w
#
# Process pins.h and generated _pins.h
#


my $input='pins.h';
my $output='pins_auto.h';

open(IN, "<$input") or die $!;
open(OUT, ">$output") or die $!;

print "Processing $input... ";
# Loop through file to process lines
while (<IN>) {
    chomp($_);
    #skip comments
    next if (/^\s*\/\/.*/);
    # parse pin definitions
    if (/^\s*PIN_DEFINE\((\w+),\s*([BCDEF]),\s*(\d)\).+/) {
	my $sym = $1;
	my $port = $2;
	my $num = $3;

	my $template =
	    "#define ${sym}_PORT\tPORT$port\n" .
	    "#define ${sym}_PIN\t_BV($num)\n" .
	    "#define ${sym}_DIR\tDDR$port\n" .
            "#define ${sym}_IN\tPIN$port\n";
	print OUT $template;
    }
}

close(IN);
close(OUT);

print "DONE\n";

