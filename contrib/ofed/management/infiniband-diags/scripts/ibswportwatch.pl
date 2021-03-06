#!/usr/bin/perl
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
use IBswcountlimits;

my $sw_addr = "";
my $sw_port = "";
my $verbose = undef;

# =========================================================================
#
sub print_verbose
{
	if ($verbose) {
		print $_[0];
	}
}

# =========================================================================
#
sub print_all_counts
{
	if (!$verbose) { return; }

	print "   Counter\t\t\tNew ==> Old\n";
	foreach my $cnt (@IBswcountlimits::counters) {
		print
"   $cnt\t\t\t$IBswcountlimits::new_counts{$cnt} ==> $IBswcountlimits::cur_counts{$cnt}\n";
	}
}

# =========================================================================
#
sub usage_and_exit
{
	my $prog = $_[0];
	print
	  "Usage: $prog [-p <pause_time> -b -v -n <cycles> -G] <guid|lid> <port>\n";
	print "   Attempt to diagnose a problem on a port\n";
	print
"   Run this on a link while a job is running which utilizes that link.\n";
	print
"   -p <pause_time> define the ammount of time between counter polls (default $IBswcountlimits::pause_time)\n";
	print "   -v Be verbose\n";
	print "   -n <cycles> run n cycles then exit (default -1 == forever)\n";
	print "   -G Address provided is a GUID\n";
	print "   -b report bytes/second packets/second\n";
	exit 2;
}

# =========================================================================
#
sub clear_counters
{
	# clear the counters
	foreach my $count (@IBswcountlimits::counters) {
		$IBswcountlimits::cur_counts{$count} = 0;
		$IBswcountlimits::new_counts{$count} = 0;
	}
}

# =========================================================================
#
sub mv_counts
{
	foreach my $count (@IBswcountlimits::counters) {
		$IBswcountlimits::cur_counts{$count} =
		  $IBswcountlimits::new_counts{$count};
	}
}

# =========================================================================
# use perfquery to get the counters.
my $GUID = "";

sub get_new_counts
{
	my $addr = $_[0];
	my $port = $_[1];
	mv_counts;
	ensure_cache_dir;
	if (
		system(
"perfquery $GUID $addr $port > $IBswcountlimits::cache_dir/perfquery.out"
		)
	  )
	{
		die "perfquery failed : \"perfquery $GUID $addr $port\"\n";
	}
	open PERF_QUERY, "<$IBswcountlimits::cache_dir/perfquery.out"
	  or die "cannot read '$IBswcountlimits::cache_dir/perfquery.out': $!\n";
	while (my $line = <PERF_QUERY>) {
		foreach my $count (@IBswcountlimits::counters) {
			if ($line =~ /^$count:\.+(\d+)/) {
				$IBswcountlimits::new_counts{$count} = $1;
			}
		}
	}
	close PERF_QUERY;
}

my $cycle = -1;    # forever

my $bytes_per_second = undef;
my $argv0            = `basename $0`;
chomp $argv0;
if (!getopts("hbvp:n:G"))        { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_h) { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_p) {
	$IBswcountlimits::pause_time = $Getopt::Std::opt_p;
}
if (defined $Getopt::Std::opt_v) { $verbose          = $Getopt::Std::opt_v; }
if (defined $Getopt::Std::opt_n) { $cycle            = $Getopt::Std::opt_n; }
if (defined $Getopt::Std::opt_G) { $GUID             = "-G"; }
if (defined $Getopt::Std::opt_b) { $bytes_per_second = $Getopt::Std::opt_b; }

my $sw_addr = $ARGV[0];
my $sw_port = $ARGV[1];

sub main
{
	clear_counters;
	get_new_counts($sw_addr, $sw_port);
	while ($cycle != 0) {
		print "Checking counts...\n";
		sleep($IBswcountlimits::pause_time);
		get_new_counts($sw_addr, $sw_port);
		check_counter_rates;
		if ($bytes_per_second) {
			print_data_rates;
		}
		print_all_counts;
		if ($cycle != -1) { $cycle = $cycle - 1; }
	}
}
main;