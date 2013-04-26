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
# $Id: socketmapServer.pl,v 1.1 2003/05/21 15:36:33 ca Exp $
use strict;
use IO::Socket;

die "usage: $0 <connection>" if (@ARGV < 1);
my $connection = shift @ARGV;
my $sock;

if ($connection =~ /tcp:(.+):([0-9]*)/) {
    $sock = new IO::Socket::INET (
				  LocalAddr => $1,
				  LocalPort => $2,
				  Proto => 'tcp',
				  Listen => 32,
				  ReuseAddr => 1
				  );
} elsif ($connection =~ /((unix)|(local)):(.+)/) {
    unlink($4);
    $sock = new IO::Socket::UNIX (
				  Type => SOCK_STREAM,
				  Local => $4,
				  Listen => 32
				  );
} else {
    die "unrecognized connection specification $connection";
}

while(my $client = $sock->accept()) {
    my $childpid = fork();
    if ($childpid) {
	$client->close();
    } else {
	die "can't fork $!" unless defined($childpid);
	$sock->close();
	handleConnection($client);
	$client->close();
	exit;
    }
}

$sock->close();

sub handleConnection {
    my $client = shift;
    $client->autoflush(1);

    while(!eof($client)) {
	eval {
	    my $request = netstringRead($client);	    
	    my ($mapName, $key) = split(' ', $request);
	    my $value = mapLookup($mapName, $key);
	    my $result = (defined($value)) ? "OK $value" : "NOTFOUND";
	    netstringWrite($client, $result);
	};
	if ($@) {
	    print STDERR "$@\n";
	    last;
	}
    }
}

sub mapLookup {
    my %mapping = ('bastiaan.bakker@example.com' => 'bastiaan', 
		   'wolter.eldering@example.com' => 'wolter@other.example.com');
    my $mapName = shift;
    my $key = shift;
    my $value = ($mapName eq "virtuser") ? $mapping{$key} : undef;
    return $value;
}

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