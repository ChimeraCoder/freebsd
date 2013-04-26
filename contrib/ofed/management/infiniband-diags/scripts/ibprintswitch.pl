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

# =========================================================================
#
sub usage_and_exit
{
	my $prog = $_[0];
	print "Usage: $prog [-R -l] [-G <switch_guid> | <switch_name>]\n";
	print "   print only the switch specified from the ibnetdiscover output\n";
	print "   -R Recalculate ibnetdiscover information\n";
	print "   -l list switches\n";
	print "   -C <ca_name> use selected channel adaptor name for queries\n";
	print "   -P <ca_port> use selected channel adaptor port for queries\n";
	print "   -G node is specified with GUID\n";
	exit 2;
}

my $argv0          = `basename $0`;
my $regenerate_map = undef;
my $list_switches  = undef;
my $ca_name        = "";
my $ca_port        = "";
my $name_is_guid   = "no";
chomp $argv0;
if (!getopts("hRlC:P:G"))         { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_h) { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_R) { $regenerate_map = $Getopt::Std::opt_R; }
if (defined $Getopt::Std::opt_l) { $list_switches  = $Getopt::Std::opt_l; }
if (defined $Getopt::Std::opt_C) { $ca_name        = $Getopt::Std::opt_C; }
if (defined $Getopt::Std::opt_P) { $ca_port        = $Getopt::Std::opt_P; }
if (defined $Getopt::Std::opt_G) { $name_is_guid   = "yes"; }

my $target_switch = $ARGV[0];

if ($name_is_guid eq "yes") {
	$target_switch = format_guid($target_switch);
}

my $cache_file = get_cache_file($ca_name, $ca_port);

if ($regenerate_map || !(-f "$cache_file")) {
	generate_ibnetdiscover_topology($ca_name, $ca_port);
}

if ($list_switches) {
	system("ibswitches $cache_file");
	exit 1;
}

if ($target_switch eq "") {
	usage_and_exit $argv0;
}

# =========================================================================
#
sub main
{
	my $found_switch = 0;
	open IBNET_TOPO, "<$cache_file" or die "Failed to open ibnet topology\n";
	my $in_switch = "no";
	my %ports     = undef;
	while (my $line = <IBNET_TOPO>) {
		if ($line =~ /^Switch.*\"S-(.*)\"\s+# (.*) port.*/) {
			my $guid = $1;
			my $desc = $2;
			if ($in_switch eq "yes") {
				$in_switch = "no";
				foreach my $port (sort { $a <=> $b } (keys %ports)) {
					print $ports{$port};
				}
			}
			if ("0x$guid" eq $target_switch || $desc =~ /[\s\"]$target_switch[\s\"]/) {
				print $line;
				$in_switch    = "yes";
				$found_switch++;
			}
		}
		if ($line =~ /^Ca.*/) { $in_switch = "no"; }

		if ($line =~ /^\[(\d+)\].*/ && $in_switch eq "yes") {
			$ports{$1} = $line;
		}

	}
	if ($found_switch == 0) {
		die "Switch \"$target_switch\" not found\n" .
			"   Try running with the \"-R\" option.\n";
	}
	if ($found_switch > 1) {
		print "\nWARNING: Found $found_switch switches with the name \"$target_switch\"\n";
	}
	close IBNET_TOPO;
}
main