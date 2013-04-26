#!/usr/bin/perl -Tw
# You may redistribute this program and/or modify it under the terms of
# the GNU General Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

use strict;
use Getopt::Std;

sub usage() {

    print(STDERR "usage: mtxstat [-gr] [-a|c|m|t] [-l limit]\n");
    exit(1);
}

MAIN:{
    my %opts;			# Command-line options
    my $key;			# Sort key
    my $limit;			# Output limit
    local *PIPE;		# Pipe
    my $header;			# Header line
    my @names;			# Field names
    my %data;			# Mutex data
    my @list;			# List of entries

    getopts("acgl:mrt", \%opts)
	or usage();
    if ($opts{'a'}) {
	usage()
	    if ($opts{'c'} || $opts{'m'} || $opts{'t'});
	$key = 'avg';
    } elsif ($opts{'c'}) {
	usage()
	    if ($opts{'m'} || $opts{'t'});
	$key = 'count';
    } elsif ($opts{'m'}) {
	usage()
	    if ($opts{'t'});
	$key = 'max';
    } elsif ($opts{'t'}) {
	$key = 'total';
    }
    if ($opts{'l'}) {
	if ($opts{'l'} !~ m/^\d+$/) {
	    usage();
	}
	$limit = $opts{'l'};
    }
    $ENV{'PATH'} = '/bin:/sbin:/usr/bin:/usr/sbin';
    open(PIPE, "sysctl -n debug.mutex.prof.stats|")
	or die("open(): $!\n");
    $header = <PIPE>;
    chomp($header);
    @names = split(' ', $header);
    if (defined($key) && !grep(/^$key$/, @names)) {
	die("can't find sort key '$key' in header\n");
    }
    while (<PIPE>) {
	chomp();
	my @fields = split(' ', $_, @names);
	next unless @fields;
	my %entry;
	foreach (@names) {
	    $entry{$_} = ($_ eq 'name') ? shift(@fields) : 0.0 + shift(@fields);
	}
	if ($opts{'g'}) {
	    $entry{'name'} =~ s/^(\S+)\s+\((.*)\)$/$2/;
	}
	my $name = $entry{'name'};
	if ($data{$name}) {
	    if ($entry{'max'} > $data{$name}->{'max'}) {
		$data{$name}->{'max'} = $entry{'max'};
	    }
	    $data{$name}->{'total'} += $entry{'total'};
	    $data{$name}->{'count'} += $entry{'count'};
	    $data{$name}->{'avg'} =
		$data{$name}->{'total'} / $data{$name}->{'count'};
	} else {
	    $data{$name} = \%entry;
	}
    }
    if (defined($key)) {
	@list = sort({ $data{$a}->{$key} <=> $data{$b}->{$key} }
		     sort(keys(%data)));
    } else {
	@list = sort(keys(%data));
    }
    if ($opts{'r'}) {
	@list = reverse(@list);
    }
    print("$header\n");
    if ($limit) {
	while (@list > $limit) {
	    pop(@list);
	}
    }
    foreach (@list) {
	printf("%6.0f %12.0f %11.0f %5.0f %-40.40s\n",
	       $data{$_}->{'max'},
	       $data{$_}->{'total'},
	       $data{$_}->{'count'},
	       $data{$_}->{'avg'},
	       $data{$_}->{'name'});
    }
}