#!/usr/bin/perl -w

package cotree;

my $class;

sub new
{
	my $type = shift();
	$class = ref($type) || $type;
	
	my $self = { left=>undef, data=>shift(), right=>undef };
	bless($self, $class);
	
	return $self;
}

sub insert
{
	my ($self, @data) = @_;
	%pr = ('=', 0, '+', 1, '*', 2);

	if (defined($data[0]{';'})) {
		#print "Detected!\n";

		return $self;
	}

	if (defined($self->{'data'}{'id'}) or defined($self->{'data'}{'num'})) {
		#print "Self is id or num.\n";

		if (defined($data[0]{'id'}) or defined($data[0]{'num'})) {
			#print "Putting id or num.\n";
			print STDERR "Achtung!\n";

			exit -1;
		} else {
			#print "Putting operator above.\n";

			$tmp = new cotree($data[0]);

			$tmp->{left} = $self;

			return $tmp;
		}
	} else {
		#print "Self is operator.\n";
		
		if (defined($data[0]{'id'}) or defined($data[0]{'num'})) {
			#print "Putting id or num to right.\n";
			
			if (defined($self->{right})) {
				#print "Shifting right.\n";

				$tmp = $self->{right}->insert($data[0]);

				$self->{right} = $tmp;
			} else {
				$self->{right} = new cotree($data[0]);
			}

			return $self;
		} else {
			#print "Listing keys...\n";

			#print keys $data[0], "\n";

			for $i (keys $data[0]) {
				for $j (keys $self->{'data'}) {
					#print $pr{$i}, "\n";
					#print $pr{$j}, "\n";

					if ($pr{$i} ge $pr{$j}) {
						if (defined($self->{right})) {
							#print "Shifting right.\n";
								
							$tmp = $self->{right}->insert($data[0]);

							$self->{right} = $tmp;
						} else {
							#print "Putting operator to right.";

							$self->{right} = new cotree($data[0]);
						}

						return $self;
					} else {
						#print '<';

						$tmp = new cotree($data[0]);

						$tmp->{left} = $self;

						return $tmp;
					}
				}
			}
		}

		return $self;
	}
}

sub printn
{
	my $self = shift();
	
	print keys $self->{data}, ' ';

	if (defined($self->{left})) {
		$self->{left}->printn();
	}

	if (defined($self->{right})) {
		$self->{right}->printn();
	} else {
		print "\n";
	}
}

sub asm
{
	my ($self, @data) = @_;

	if (defined($self->{right})) {
		$self->{right}->asm(@data);
	}

	if (defined($self->{left})) {
		$self->{left}->asm(@data);
	}

	if (defined($self->{'data'}{'id'})) {
		open $handle, ">>main.asm";

		print $handle "\tpush $data[$self->{'data'}{'id'}]{'value'}\n";
		
		close $handle;
	} elsif (defined($self->{'data'}{'num'})) {
		open $handle, ">>main.asm";

		print $handle "\tpush $data[$self->{'data'}{'num'}]{'value'}\n";
		
		close $handle;
	} else {
		open $handle, ">>main.asm";

		print $handle "\tpop rax\n";
		print $handle "\tpop rbx\n";

		SWITCH: {
		if (defined($self->{data}{'+'})) { print $handle "\tadd rax, rbx\n"; last SWITCH; }
		if (defined($self->{data}{'*'})) { print $handle "\tmul rbx\n"; last SWITCH; }
		if (defined($self->{data}{'='})) { print $handle "\tmov rax, rbx\n"; last SWITCH; }
		}

		print $handle "\tpush rax\n";
		
		close $handle;
	}	
}

return "true";
