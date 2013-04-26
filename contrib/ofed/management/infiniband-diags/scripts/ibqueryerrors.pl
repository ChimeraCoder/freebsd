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

my $print_action          = "no";
my $report_port_info      = undef;
my $single_switch         = undef;
my $include_data_counters = undef;
my $cache_file            = "";
my $switch_found          = "no";

# =========================================================================
#
sub report_counts
{
	my $addr         = $_[0];
	my $port         = $_[1];
	my $ca_name      = $_[2];
	my $ca_port      = $_[3];
	my $extra_params = get_ca_name_port_param_string($ca_name, $ca_port);

	if (any_counts()) {
		print("   GUID $addr port $port:");
		check_counters($print_action);
		if ($include_data_counters) {
			check_data_counters($print_action);
		}
		print("\n");

		if ($report_port_info) {
			my $lid   = "";
			my $speed = "";
			my $width = "";
			my $data  = `smpquery $extra_params -G portinfo $addr $port`;
			my @lines = split("\n", $data);
			foreach my $line (@lines) {
				if ($line =~ /^# Port info: Lid (\w+) port.*/) { $lid   = $1; }
				if ($line =~ /^LinkSpeedActive:\.+(.*)/)       { $speed = $1; }
				if ($line =~ /^LinkWidthActive:\.+(.*)/)       { $width = $1; }
			}
			my $hr = $IBswcountlimits::link_ends{"$addr"}{$port};
			if ($hr) {
				printf(
"         Link info: %6s %4s[%2s]  ==(%3s %s)==>  %18s %4s[%2s] \"%s\"\n",
					$lid,                $port,
					$hr->{loc_ext_port}, $width,
					$speed,              $hr->{rem_guid},
					$hr->{rem_port},     $hr->{rem_ext_port},
					$hr->{rem_desc}
				);
			} else {
				printf(
"         Link info: %6s %4s[  ]  ==(%3s %s)==>     (Disconnected)\n",
					$lid, $port, $width, $speed);
			}
		}
	}
}

# =========================================================================
# use perfquery to get the counters.
sub get_counts
{
	my $addr         = $_[0];
	my $port         = $_[1];
	my $ca_name      = $_[2];
	my $ca_port      = $_[3];
	my $extra_params = get_ca_name_port_param_string($ca_name, $ca_port);

	my $data = `perfquery $extra_params -G $addr $port` ||
		die "'perfquery $extra_params -G $addr $port' FAILED.\n";
	my @lines = split("\n", $data);
	foreach my $line (@lines) {
		foreach my $count (@IBswcountlimits::counters) {
			if ($line =~ /^$count:\.+(\d+)/) {
				$IBswcountlimits::cur_counts{$count} = $1;
			}
		}
	}
}

# =========================================================================
#
my %switches = ();

sub get_switches
{
	my $data = `ibswitches $cache_file` ||
		die "'ibswitches $cache_file' failed.\n";
	my @lines = split("\n", $data);
	foreach my $line (@lines) {
		if ($line =~ /^Switch\s+:\s+(\w+)\s+ports\s+(\d+)\s+.*/) {
			$switches{$1} = $2;
		}
	}
}

# =========================================================================
#
sub usage_and_exit
{
	my $prog = $_[0];
	print
"Usage: $prog [-a -c -r -R -s <err1,err2,...> -S <switch_guid> -D <direct route> -d -C <ca_name> -P <ca_port>]\n";
	print "   Report counters on all switches in subnet\n";
	print "   -a Report an action to take\n";
	print "   -c suppress some of the common counters\n";
	print "   -r report port configuration information\n";
	print "   -R Recalculate ibnetdiscover information\n";
	print "   -s <err1,err2,...> suppress errors listed\n";
	print
"   -D <direct route> output only the switch specified by direct route path\n";
	print "   -S <switch_guid> query only <switch_guid> (hex format)\n";
	print "   -d include the data counters in the output\n";
	print "   -C <ca_name> use selected Channel Adaptor name for queries\n";
	print "   -P <ca_port> use selected channel adaptor port for queries\n";
	exit 2;
}

my $argv0          = `basename $0`;
my $regenerate_map = undef;
my $single_switch  = undef;
my $direct_route   = undef;
my $ca_name        = "";
my $ca_port        = "";

chomp $argv0;
if (!getopts("has:crRS:D:dC:P:")) { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_h)  { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_a) { $print_action = "yes"; }
if (defined $Getopt::Std::opt_s) {
	@IBswcountlimits::suppress_errors = split(",", $Getopt::Std::opt_s);
}
if (defined $Getopt::Std::opt_c) {
	@IBswcountlimits::suppress_errors = split(",", "RcvSwRelayErrors");
}
if (defined $Getopt::Std::opt_r) { $report_port_info = $Getopt::Std::opt_r; }
if (defined $Getopt::Std::opt_R) { $regenerate_map   = $Getopt::Std::opt_R; }
if (defined $Getopt::Std::opt_D) { $direct_route     = $Getopt::Std::opt_D; }
if (defined $Getopt::Std::opt_S) {
	$single_switch = format_guid($Getopt::Std::opt_S);
}
if (defined $Getopt::Std::opt_d) {
	$include_data_counters = $Getopt::Std::opt_d;
}
if (defined $Getopt::Std::opt_C) { $ca_name = $Getopt::Std::opt_C; }
if (defined $Getopt::Std::opt_P) { $ca_port = $Getopt::Std::opt_P; }

$cache_file = get_cache_file($ca_name, $ca_port);

sub main
{
	if (@IBswcountlimits::suppress_errors) {
		my $msg = join(",", @IBswcountlimits::suppress_errors);
		print "Suppressing: $msg\n";
	}
	get_link_ends($regenerate_map, $ca_name, $ca_port);
	get_switches;
	if (defined($direct_route)) {
		# convert DR to guid, then use original single_switch option
		$single_switch = convert_dr_to_guid($direct_route);
		if (!defined($single_switch) || !is_switch($single_switch)) {
			printf("The direct route (%s) does not map to a switch.\n",
				$direct_route);
			return;
		}
	}
	foreach my $sw_addr (keys %switches) {
		if ($single_switch && $sw_addr ne "$single_switch") {
			next;
		} else {
			$switch_found = "yes";
		}

		my $switch_prompt = "no";
		foreach my $sw_port (1 .. $switches{$sw_addr}) {
			clear_counters;
			get_counts($sw_addr, $sw_port, $ca_name, $ca_port);
			if (any_counts() && $switch_prompt eq "no") {
				my $hr = $IBswcountlimits::link_ends{"$sw_addr"}{$sw_port};
				printf("Errors for %18s \"%s\"\n", $sw_addr, $hr->{loc_desc});
				$switch_prompt = "yes";
			}
			report_counts($sw_addr, $sw_port);
		}
	}
	if ($single_switch && $switch_found ne "yes") {
		printf("Switch \"%s\" not found.\n", $single_switch);
	}
}
main;