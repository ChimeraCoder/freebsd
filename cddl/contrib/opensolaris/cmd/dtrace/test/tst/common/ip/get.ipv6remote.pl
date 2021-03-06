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
# get.ipv6remote.pl
#
# Find an IPv6 reachable remote host using both ifconfig(1M) and ping(1M).
# Print the local address and the remote address, or print nothing if either
# no IPv6 interfaces or remote hosts were found.  (Remote IPv6 testing is
# considered optional, and so not finding another IPv6 host is not an error
# state we need to log.)  Exit status is 0 if a host was found.
#

use strict;
use IO::Socket;

my $MAXHOSTS = 32;			# max hosts to scan
my $TIMEOUT = 3;			# connection timeout
my $MULTICAST = "FF02::1";		# IPv6 multicast address

#
# Determine local IP address
#
my $local = "";
my $remote = "";
my %Local;
my $up;
open IFCONFIG, '/sbin/ifconfig -a inet6 |'
    or die "Couldn't run ifconfig: $!\n";
while (<IFCONFIG>) {
	next if /^lo/;

	# "UP" is always printed first (see print_flags() in ifconfig.c):
	$up = 1 if /^[a-z].*<UP,/;
	$up = 0 if /^[a-z].*<,/;

	# assume output is "inet6 ...":
	if (m:inet6 (\S+)/:) {
		my $addr = $1;
                $Local{$addr} = 1;
                $local = $addr if $up and $local eq "";
		$up = 0;
	}
}
close IFCONFIG;
exit 1 if $local eq "";

#
# Find the first remote host that responds to an icmp echo,
# which isn't a local address.
#
open PING, "/sbin/ping -ns -A inet6 $MULTICAST 56 $MAXHOSTS |" or
    die "Couldn't run ping: $!\n";
while (<PING>) {
	if (/bytes from (.*): / and not defined $Local{$1}) {
		$remote = $1;
		last;
	}
}
close PING;
exit 2 if $remote eq "";

print "$local $remote\n";