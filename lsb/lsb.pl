#!/usr/bin/perl -w

sub incept
{
	print "Modify:\n";

	for ($i = 0, $j = 0; $j < $#bit + 1; $i++, $j++) {
		if (($i % $string) eq $w) {
			$i += $shift;
		}

		printf "%02x ", $buf[$offset + $i];
	}
	print "\n";

	print "Modified:\n";

	open FILE, ">output.bmp";

	for ($i = 0; $i < $offset; $i++) {
		print FILE chr $buf[$i];
	}

	for ($i = 0, $j = 0; $i + $offset < $#buf + 1; $i++, $j++) {
		if (($i % $string) eq $w) {
			$i += $shift;
		}

		if ($bit[$j]) {
			$buf [$offset + $i] = $buf[$offset + $i] | $bit[$j];
		} else {
			$buf[$offset + $i] = $buf[$offset + $i] >> 1;
			$buf[$offset + $i] = $buf[$offset + $i] << 1;
		}

		if ($j < $#bit + 1) {
			printf "%02x ", $buf[$offset + $i];
		} else {
			#printf "++ ";
		}

		print FILE chr $buf[$i + $offset];
	}
	print "\n";

	close FILE;
}

sub inspect
{
	$offset = $buf[10] + $buf[11] * 16 + $buf[12] * 16 ** 2 + $buf[13] * 16 ** 3;
	print '$offset = ', $offset, " byte(s)\n";

	$width = $buf[18] + $buf[19] * 16 + $buf[20] * 16 ** 2 + $buf[21] * 16 ** 3;
	print '$width = ', $width, " pixel(s)\n";

	$height = $buf[22] + $buf[23] * 16 + $buf[24] * 16 ** 2 + $buf[25] * 16 ** 3;
	print '$height = ', $height, " pixel(s)\n";

	print '$#inputbuf + 1 = ', $#inputbuf + 1, " byte(s)\n";

	$size = ($width * $height * 3) / 8;
	print '$size = ', $size, " byte(s)\n";

	$w = $width * 3;
	print '$w = ', $w, " byte(s)\n";

	$shift = $w % 4;
	print '$shift = ', $shift, " byte(s)\n";

	if ($shift) {
		$string = $w + (4 - $shift);
	} else {
		$string = $w;
	}
	print '$string = ', $string, " byte(s)\n";

	$shift = $string - $w;

	print '$shift = ', $shift, " byte(s)\n";
}

sub parse_container
{
	$/ = undef;
	$buf = <>;
	@_buf = split (//, $buf);

	for ($i = 0; $i < $#_buf + 1; $i++) {
		$buf[$i] = ord ($_buf[$i]);
	}
}

sub parse_input
{
	$input = "Hello, world!";

	@_inputbuf = split (//, $input);

	for ($i = 0; $i < $#_inputbuf + 1; $i++) {
		$inputbuf[$i] = ord ($_inputbuf[$i]);
	}

	#for ($i = 0; $i < $#_inputbuf + 1; $i++) {
	#	printf "%2s ", $_inputbuf[$i];
	#}
	#print "\n";
	#
	#for ($i = 0; $i < $#inputbuf + 1; $i++) {
	#	printf "%2x ", $inputbuf[$i];
	#}
	#print "\n";

	for ($i = 0; $i < $#inputbuf + 1; $i++) {
		printf "%x = ", $inputbuf[$i];

		for ($j = 7; $j >= 0; $j--) {
			$bit[$i * 8 + $j] = ($inputbuf[$i] & (2 ** $j));
			print $bit[$i * 8 + $j] = $bit[$i * 8 + $j] >> $j;
		}

		print "\n";
	}
}

if (!@ARGV) {
	print "lsb: You must specify one of the '-ix' options\n";

	exit -1;
} else {
	@args = split (//, $ARGV[0]);

	#for ($i = 0; $i < $#args + 1; $i++) {
	#	print $args[$i], "\n";
	#}

	if ($args[0] eq "-") {
		SWITCH: {
			if ($ARGV[0] =~ /i/) {
				shift @ARGV;

				parse_container();

				parse_input();

				inspect();

				if ($#inputbuf + 1 > $size) {
					print '$#inputbuf + 1 > $size', "\n";

					exit -1;
				}

				incept();

				last SWITCH;
			} elsif ($ARGV[0] =~ /x/) { shift @ARGV; parse_container(); inspect(); last SWITCH; }

			print "lsb: invalid option -- '$ARGV[0]'\n";

			exit -1;
		}
	} else {
		print "lsb: invalid option -- '$ARGV[0]'\n";

		exit -1;
	}
}
#} elsif ($ARGV[0] eq "-i") {
#	if (!$ARGV[1]) {
#		print "lsb: option requires an argument -- '$ARGV[0]'\n";
#
#		exit -1;
#	}
#
#	shift @ARGV;
#
#	parse_container();
#
#	parse_input();
#} elsif ($ARGV[0] eq "-x") {
#	if (!$ARGV[1]) {
#		print "lsb: option requires an argument -- '$ARGV[0]'\n";
#
#		exit -1;
#	}
#
#	shift @ARGV;
#
#	parse_container();
#} else {
#	print "lsb: invalid option -- '$ARGV[0]'\n";
#
#	exit -1;
#}

#for ($i = $offset; $i < $#buf + 1; $i++) {
#	printf "%02x ", $buf[$i];
#}
#print "\n";
