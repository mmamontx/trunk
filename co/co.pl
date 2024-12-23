#!/usr/bin/perl -w

use cotree;

open $fh, "<input.c";
#$/ = undef;
$buf = <$fh>;
close $fh;

chop $buf;

@var = split (/\W/, $buf);

@op = split (/\w+/, $buf);
shift @op;

$count = 0;

for $i (0 .. $#var + $#op + 1) {
	if($i % 2) {
		if (@op) {
			$tok[$i]{"$op[0]"} = "";

			shift @op;
		}
	} elsif (@var) {
		if ($var[0] =~ /\D/) {
			$sym[$count]{"name"} = $var[0];
			$sym[$count]{"value"} = 0;

			$tok[$i]{"id"} = $count++;
		} else {
			$sym[$count]{"value"} = $var[0];

			$tok[$i]{"num"} = $count++;
		}

		shift @var;
	}	
}

for $href (@tok) {
	print "\<";
	
	for $role (keys %$href) {
		print "$role, $href->{$role}";
	}

	print "\>\n";
}
print "\n";

my $tree = new cotree($tok[0]);

for $i (1 .. $#tok) {
	#print '(', $i, ")\n";
	$tree = $tree->insert($tok[$i]);
	#print "\n";
}

$tree->printn();

print "\n";

$tree->asm(@sym);
