#!/usr/bin/perl -w
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

sub EMPTY() {}

MAIN:{
    my %opts;
    getopts('c', \%opts);

    my %config;
    my $machine;
    my $ident;

    while (<>) {
	chomp();
	s/\s*(\#.*)?$//;
	next unless $_;
	my ($keyword, $values) = split(' ', $_, 2);
	foreach my $value (split(/,\s*/, $values)) {
	    if ($keyword eq 'machine') {
		$machine = $value;
	    } elsif ($keyword eq 'ident') {
		$ident = $value;
	    } elsif ($keyword eq 'options' && $value =~ m/(\w+)=(.+)/) {
		$config{$keyword}->{$1} = $2;
	    } else {
		$config{$keyword}->{$value} = \&EMPTY;
	    }
	}
    }

    my $generic;
    if ($machine) {
	$generic = "/usr/src/sys/$machine/conf/GENERIC";
    } else {
	($generic = $ARGV) =~ s|([^/])+$|GENERIC|;
    }
    local *GENERIC;
    open(GENERIC, "<", $generic)
	or die("$generic: $!\n");
    my $blank = 0;
    while (<GENERIC>) {
	my $line = $_;
	chomp();
	if ($opts{'c'} && m/^\s*\#/) {
	    if ($blank) {
		print "\n";
		$blank = 0;
	    }
	    print $line;
	    next;
	}
	++$blank unless $_;
	s/\s*(\#.*)?$//;
	next unless $_;
	my ($keyword, $value) = split(' ', $_);
	if ($keyword eq 'machine') {
	    die("$generic is for $value, not $machine\n")
		unless ($value eq $machine);
	} elsif ($keyword eq 'ident') {
	    $line =~ s/$value/$ident/;
	} elsif ($keyword eq 'options' && $value =~ m/(\w+)=(.+)/ &&
	    $config{$keyword} && $config{$keyword}->{$1} &&
	    $config{$keyword}->{$1} != \&EMPTY) {
	    $value = $1;
	    if ($config{$keyword}->{$value} ne $2) {
		my ($old, $new) = ($2, $config{$keyword}->{$value});
		$line =~ s{=$old}{=$new};
	    }
	    delete($config{$keyword}->{$value});
	    delete($config{$keyword})
		unless %{$config{$keyword}};
	} elsif ($config{$keyword} && $config{$keyword}->{$value}) {
	    delete($config{$keyword}->{$value});
	    delete($config{$keyword})
		unless %{$config{$keyword}};
	} else {
	    next;
	}
	if ($blank) {
	    print "\n";
	    $blank = 0;
	}
	print $line;
    }
    close(GENERIC);

    if (%config) {
	print "\n# Addenda\n";
	foreach my $keyword (sort(keys(%config))) {
	    foreach my $value (sort(keys(%{$config{$keyword}}))) {
		print "$keyword";
		if (length($keyword) < 7) {
		    print "\t";
		} elsif (length($keyword) == 7) {
		    print " ";
		}
		print "\t$value";
		print "=$config{$keyword}->{$value}"
		    unless $config{$keyword}->{$value} == \&EMPTY;
		print "\n";
	    }
	}
    }
    exit(0);
}