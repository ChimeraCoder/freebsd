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

sub usage_and_exit
{
	my $prog = $_[0];
	print "Usage: $prog [-Rh]\n";
	print
"   Validate LIDs and GUIDs (check for zero and duplicates) in the local subnet\n";
	print "   -h This help message\n";
	print
"   -R Recalculate ibnetdiscover information (Default is to reuse ibnetdiscover output)\n";
	exit 2;
}

my $argv0          = `basename $0`;
my $regenerate_map = undef;

chomp $argv0;
if (!getopts("hR")) { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_h) { usage_and_exit $argv0; }
if (defined $Getopt::Std::opt_R) { $regenerate_map = $Getopt::Std::opt_R; }

sub validate_non_zero_lid
{
	my ($lid)      = shift(@_);
	my ($nodeguid) = shift(@_);
	my ($nodetype) = shift(@_);

	if ($lid eq 0) {
		print "LID 0 found for $nodetype NodeGUID $nodeguid\n";
		return 1;
	}
	return 0;
}

sub validate_non_zero_guid
{
	my ($lid)      = shift(@_);
	my ($guid)     = shift(@_);
	my ($nodetype) = shift(@_);

	if ($guid eq 0x0) {
		print "$nodetype GUID 0x0 found with LID $lid\n";
		return 1;
	}
	return 0;
}

$insert_lid::lids           = undef;
$insert_nodeguid::nodeguids = undef;
$insert_portguid::portguids = undef;

sub insert_lid
{
	my ($lid)      = shift(@_);
	my ($nodeguid) = shift(@_);
	my ($nodetype) = shift(@_);
	my $rec        = undef;
	my $status     = "";

	$status = validate_non_zero_lid($lid, $nodeguid, $nodetype);
	if ($status eq 0) {
		if (defined($insert_lid::lids{$lid})) {
			print
"LID $lid already defined for NodeGUID $insert_lid::lids{$lid}->{nodeguid}\n";
		} else {
			$rec = {lid => $lid, nodeguid => $nodeguid};
			$insert_lid::lids{$lid} = $rec;
		}
	}
}

sub insert_nodeguid
{
	my ($lid)      = shift(@_);
	my ($nodeguid) = shift(@_);
	my ($nodetype) = shift(@_);
	my $rec        = undef;
	my $status     = "";

	$status = validate_non_zero_guid($lid, $nodeguid, $nodetype);
	if ($status eq 0) {
		if (defined($insert_nodeguid::nodeguids{$nodeguid})) {
			print
"NodeGUID $nodeguid already defined for LID $insert_nodeguid::nodeguids{$nodeguid}->{lid}\n";
		} else {
			$rec = {lid => $lid, nodeguid => $nodeguid};
			$insert_nodeguid::nodeguids{$nodeguid} = $rec;
		}
	}
}

sub validate_portguid
{
	my ($portguid)  = shift(@_);
	my ($firstport) = shift(@_);

	if (defined($insert_nodeguid::nodeguids{$portguid})
		&& ($firstport ne "yes"))
	{
		print "PortGUID $portguid is invalid duplicate of a NodeGUID\n";
	}
}

sub insert_portguid
{
	my ($lid)       = shift(@_);
	my ($portguid)  = shift(@_);
	my ($nodetype)  = shift(@_);
	my ($firstport) = shift(@_);
	my $rec         = undef;
	my $status      = "";

	$status = validate_non_zero_guid($lid, $portguid, $nodetype);
	if ($status eq 0) {
		if (defined($insert_portguid::portguids{$portguid})) {
			print
"PortGUID $portguid already defined for LID $insert_portguid::portguids{$portguid}->{lid}\n";
		} else {
			$rec = {lid => $lid, portguid => $portguid};
			$insert_portguid::portguids{$portguid} = $rec;
			validate_portguid($portguid, $firstport);
		}
	}
}

sub main
{
	if ($regenerate_map
		|| !(-f "$IBswcountlimits::cache_dir/ibnetdiscover.topology"))
	{
		generate_ibnetdiscover_topology;
	}

	open IBNET_TOPO, "<$IBswcountlimits::cache_dir/ibnetdiscover.topology"
	  or die "Failed to open ibnet topology: $!\n";

	my $nodetype  = "";
	my $nodeguid  = "";
	my $portguid  = "";
	my $lid       = "";
	my $line      = "";
	my $firstport = "";

	while ($line = <IBNET_TOPO>) {

		if ($line =~ /^caguid=(.*)/ || $line =~ /^rtguid=(.*)/) {
			$nodeguid = $1;
			$nodetype = "";
		}

		if ($line =~ /^switchguid=(.*)/) {
			$nodeguid = $1;
			$portguid = "";
			$nodetype = "";
		}
		if ($line =~ /^switchguid=(.*)\((.*)\)/) {
			$nodeguid = $1;
			$portguid = "0x" . $2;
		}

		if ($line =~ /^Switch.*\"S-(.*)\"\s+# (.*) port.* lid (\d+) .*/) {
			$nodetype  = "switch";
			$firstport = "yes";
			$lid       = $3;
			insert_lid($lid, $nodeguid, $nodetype);
			insert_nodeguid($lid, $nodeguid, $nodetype);
			if ($portguid ne "") {
				insert_portguid($lid, $portguid, $nodetype, $firstport);
			}
		}
		if ($line =~ /^Ca.*/) {
			$nodetype  = "ca";
			$firstport = "yes";
		}
		if ($line =~ /^Rt.*/) {
			$nodetype  = "router";
			$firstport = "yes";
		}

		if ($nodetype eq "ca" || $nodetype eq "router") {
			if ($line =~ /"S-(.*)\# lid (\d+) .*/) {
				$lid = $2;
				insert_lid($lid, $nodeguid, $nodetype);
				if ($firstport eq "yes") {
					insert_nodeguid($lid, $nodeguid, $nodetype);
					$firstport = "no";
				}
			}
			if ($line =~ /^.*"H-(.*)\# lid (\d+) .*/) {
				$lid = $2;
				insert_lid($lid, $nodeguid, $nodetype);
				if ($firstport eq "yes") {
					insert_nodeguid($lid, $nodeguid, $nodetype);
					$firstport = "no";
				}
			}
			if ($line =~ /^.*"R-(.*)\# lid (\d+) .*/) {
				$lid = $2;
				insert_lid($lid, $nodeguid, $nodetype);
				if ($firstport eq "yes") {
					insert_nodeguid($lid, $nodeguid, $nodetype);
					$firstport = "no";
				}
			}
			if ($line =~ /^\[(\d+)\]\((.*)\)/) {
				$portguid = "0x" . $2;
				insert_portguid($lid, $portguid, $nodetype, $firstport);
			}
		}

	}

	close IBNET_TOPO;
}
main;