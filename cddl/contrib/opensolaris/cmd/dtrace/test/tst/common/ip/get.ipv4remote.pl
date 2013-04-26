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

#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
#pragma ident	"%Z%%M%	%I%	%E% SMI"

#
# get.ipv4remote.pl [tcpport]
#
# Find an IPv4 reachable remote host using both ifconfig(1M) and ping(1M).
# If a tcpport is specified, return a host that is also listening on this
# TCP port.  Print the local address and the remote address, or an
# error message if no suitable remote host was found.  Exit status is 0 if
# a host was found.
#

use strict;
use IO::Socket;

my $MAXHOSTS = 32;			# max hosts to port scan
my $TIMEOUT = 3;			# connection timeout
my $tcpport = @ARGV == 1 ? $ARGV[0] : 0;

#
# Determine local IP address
#
my $local = "";
my $remote = "";
my %Broadcast;
my $up;
open IFCONFIG, '/sbin/ifconfig -a |' or die "Couldn't run ifconfig: $!\n";
while (<IFCONFIG>) {
	next if /^lo/;

	# "UP" is always printed first (see print_flags() in ifconfig.c):
	$up = 1 if /^[a-z].*<UP,/;
	$up = 0 if /^[a-z].*<,/;

	# assume output is "inet X ... broadcast Z":
	if (/inet (\S+) .* broadcast (\S+)/) {
		my ($addr, $bcast) = ($1, $2);
		$Broadcast{$addr} = $bcast;
		$local = $addr if $up and $local eq "";
		$up = 0;
	}
}
close IFCONFIG;
die "Could not determine local IP address" if $local eq "";

#
# Find the first remote host that responds to an icmp echo,
# which isn't a local address.
#
open PING, "/sbin/ping -ns $Broadcast{$local} 56 $MAXHOSTS |" or
    die "Couldn't run ping: $!\n";
while (<PING>) {
	if (/bytes from (.*): / and not defined $Broadcast{$1}) {
		my $addr = $1;

		if ($tcpport != 0) {
			#
			# Test TCP
			#
			my $socket = IO::Socket::INET->new(
				Proto    => "tcp",
				PeerAddr => $addr,
				PeerPort => $tcpport,
				Timeout  => $TIMEOUT,
			);
			next unless $socket;
			close $socket;
		}

		$remote = $addr;
		last;
	}
}
close PING;
die "Can't find a remote host for testing: No suitable response from " .
    "$Broadcast{$local}\n" if $remote eq "";

print "$local $remote\n";