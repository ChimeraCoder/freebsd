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
# Contributed by Bastiaan Bakker for SOCKETMAP
# $Id: socketmapClient.pl,v 1.1 2003/05/21 15:36:33 ca Exp $
use strict;
use IO::Socket;

die "usage: $0 <connection> <mapname> <key> [<key2> ...]" if (@ARGV < 3);

my $connection = shift @ARGV;
my $mapname = shift @ARGV;

my $sock;

if ($connection =~ /tcp:(.+):([0-9]*)/) {
    $sock = new IO::Socket::INET (
				  PeerAddr => $1,
				  PeerPort => $2,
				  Proto => 'tcp',
				  );
} elsif ($connection =~ /((unix)|(local)):(.+)/) {
    $sock = new IO::Socket::UNIX (
				  Type => SOCK_STREAM,
				  Peer => $4
				  );
} else {
    die "unrecognized connection specification $connection";
}

die "Could not create socket: $!\n" unless $sock;

while(my $key = shift @ARGV) {
    my $request = "$mapname $key";
    netstringWrite($sock, $request);
    $sock->flush();
    my $response = netstringRead($sock);

    print "$key => $response\n";
}

$sock->close();

sub netstringWrite {
    my $sock = shift;
    my $data = shift;

    print $sock length($data).':'.$data.',';
}

sub netstringRead {
    my $sock = shift;
    my $saveSeparator = $/;
    $/ = ':';
    my $dataLength = <$sock>;
    die "cannot read netstring length" unless defined($dataLength);
    chomp $dataLength;
    my $data;
    if ($sock->read($data, $dataLength) == $dataLength) {
	($sock->getc() eq ',') or die "data misses closing ,";
    } else {
	die "received only ".length($data)." of $dataLength bytes";
    }
    
    $/ = $saveSeparator;
    return $data;
}