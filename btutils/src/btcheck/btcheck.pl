sub csum {
	local $csum = 0;
	
	if (@_) {
		local $buf = 0;
		local $_buf = 0;
		
		open $fh, "<@_";
		
		binmode $fh;
		
		$buf = <$fh>;
		close $fh;
		@_buf = split (//, $buf);
		
		for ($i = 0; $i < $#_buf + 1; $i++) {
			$buf[$i] = ord ($_buf[$i]);
		}
	}
	
	for ($csum = $i = 0; $i + 8 < $#buf + 1; $i++) {
		$csum ^= $buf[$i + 8];
		$csum = ($csum >> 8) ^ $table[$csum & 0xff];
	}
	
	$csum = 0xffff & (($csum >> 8) ^ ($csum << 8));
	
	if (@_) {
		if ($csum != $buf[5] * 16 ** 2 + $buf[4]) {
			print '$csum != $buf[5] * 16 ** 2 + $buf[4]', "\n";
			
			exit -1;
		} else {
			print "@_ ";
			printf "%02x%02x ", $buf[1], $buf[0];
			printf "%02x%02x\n", $buf[5], $buf[4];
		}
	}

	printf "%04x\n", $csum;
}

if (!@ARGV) {
	print "ver: fatal error: no input files\n";
	
	exit -1;
}

@arr = ("almanach\.arc", "pz", "pzk\.bin", "rzk\.bin");

for ($i = 0; $i < 256; $i++) {
	$tmp = $i << 8;
	
	for ($j = 0; $j < 8; $j++) {
		$_tmp = $tmp & 0x8000;
		$_tmp = -(($_tmp) != 0);
		$tmp <<= 1;
		$tmp ^= $_tmp & hex 1021;
		$tmp &= 0xffff;
	}
	
	$table[$i] = ($tmp << 8) ^ ($tmp >> 8);
	$table[$i] &= 0xffff;
}

$/ = undef;
$buf = <>;
close $ARGV;
@_buf = split (//, $buf);

for ($i = 0; $i < $#_buf + 1; $i++) {
	$buf[$i] = ord ($_buf[$i]);
}

printf "%02x%02x%02x%02x ", $buf[3], $buf[2], $buf[1], $buf[0];
printf "%02x%02x\n", $buf[5], $buf[4];

$csum = $buf[5] * 16 ** 2 + $buf[4];

csum();

if ($ARGV =~ /vci\.bin/) {
	$tmp = 0;
	
	if ($buf[42] * 16 ** 2 + $buf[43]) {
		$tmp++;
	}
	
	if ($buf[54] * 16 ** 2 + $buf[55]) {
		$tmp++;
	}
	
	if ($tmp == 2) {
		csum($` . "pzk.bin");
		
		csum($` . "rzk.bin");
	} elsif ($tmp == 1) {
		print '$tmp == 1\n', "\n";
		
		exit -1;
	}
} else {
	LOOP: for ($k = 0; $k < $#arr + 1; $k++) {
		if ($ARGV =~ /$arr[$k]/) {
			if ($') {
				next;
			}

			$tmp = $buf[3] * 16 ** 4 + $buf[2] * 16 ** 3 + $buf[1] * 16 ** 2 + $buf[0];
			
			for ($i = 0; $i * 94 + 8 + $i < $tmp; $i += 2) {
				for ($j = $i * 94 + $i, $csum = 0; $j < $i * 94 + $i + 188; $j += 2) {
					$csum += $buf[$j + 8] * 16 ** 2 + $buf[$j + 8 + 1];
				}
				
				$_tmp = $buf[$i * 94 + 188 + 8 + $i] * 16 ** 2 + $buf[$i * 94 + 188 + 8 + $i + 1];
				
				if (($csum &= 0xffff) != $_tmp) {
					print '($csum &= 0xffff) != $_tmp', "\n";
					
					exit -1;
				}
			}

			printf "Successfully checked %d of %d blocks.\n", $i, $i;
			
			last LOOP;
		}
	}
}
