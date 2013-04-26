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
	print "Usage: $prog [-R -l] [-G <ca_guid> | <node_name>]\n";
	print "   print only the ca specified from the ibnetdiscover output\n";
	print "   -R Recalculate ibnetdiscover information\n";
	print "   -l list cas\n";
	print "   -C <ca_name> use selected channel adaptor name for queries\n";
	print "   -P <ca_port> use selected channel adaptor port for queries\n";
	print "   -G node is specified with GUID\n";
	exit 2;
}

my $argv0          = `basename $0`;
my $regenerate_map = undef;
my $list_hcas      = undef;
my $ca_name        = "";
my $ca_port        = "";
my $name_is_guid   = "no";
chomp $argv0;
if (!getopts("hRlC:P:G"))         { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_h) { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_R) { $regenerate_map = $Getopt::Std::opt_R; }
if (defined $Getopt::Std::opt_l) { $list_hcas      = $Getopt::Std::opt_l; }
if (defined $Getopt::Std::opt_C) { $ca_name        = $Getopt::Std::opt_C; }
if (defined $Getopt::Std::opt_P) { $ca_port        = $Getopt::Std::opt_P; }
if (defined $Getopt::Std::opt_G) { $name_is_guid   = "yes"; }

my $target_hca = $ARGV[0];

if ($name_is_guid eq "yes") {
	$target_hca = format_guid($target_hca);
}

my $cache_file = get_cache_file($ca_name, $ca_port);

if ($regenerate_map || !(-f "$cache_file")) {
	generate_ibnetdiscover_topology($ca_name, $ca_port);
}

if ($list_hcas) {
	system("ibhosts $cache_file");
	exit 1;
}

if ($target_hca eq "") {
	usage_and_exit $argv0;
}

# =========================================================================
#
sub main
{
	my $found_hca = 0;
	open IBNET_TOPO, "<$cache_file" or die "Failed to open ibnet topology\n";
	my $in_hca = "no";
	my %ports  = undef;
	while (my $line = <IBNET_TOPO>) {
		if ($line =~ /^Ca.*\"H-(.*)\"\s+# (.*)/) {
			my $guid = $1;
			my $desc = $2;
			if ($in_hca eq "yes") {
				$in_hca = "no";
				foreach my $port (sort { $a <=> $b } (keys %ports)) {
					print $ports{$port};
				}
			}
			if ("0x$guid" eq $target_hca || $desc =~ /[\s\"]$target_hca[\s\"]/) {
				print $line;
				$in_hca    = "yes";
				$found_hca++;
			}
		}
		if ($line =~ /^Switch.*/ || $line =~ /^Rt.*/) { $in_hca = "no"; }

		if ($line =~ /^\[(\d+)\].*/ && $in_hca eq "yes") {
			$ports{$1} = $line;
		}

	}
	if ($found_hca == 0) {
		die "\"$target_hca\" not found\n" .
			"   Try running with the \"-R\" option.\n" .
			"   If still not found the node is probably down.\n";
	}
	if ($found_hca > 1) {
		print "\nWARNING: Found $found_hca CA's with the name \"$target_hca\"\n";
	}
	close IBNET_TOPO;
}
main