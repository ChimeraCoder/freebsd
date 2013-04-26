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
use Fcntl;
use vars qw(%SERVICES);

MAIN:{
    my $line;
    my $service;
    my $version;
    my $type;
    local *FILE;

    while (<>) {
	chomp();
	s/\s*$//;
	next unless m/^(\#*)(\w+)\s+(auth|account|session|password)\s+(\S.*)$/;
	$line = $1.$3;
	$line .= "\t" x ((16 - length($line) + 7) / 8);
	$line .= $4;
	push(@{$SERVICES{$2}->{$3}}, $line);
    }

    foreach $service (keys(%SERVICES)) {
	$version = '$' . 'FreeBSD' . '$';
	if (sysopen(FILE, $service, O_RDONLY)) {
		while (<FILE>) {
			next unless (m/(\$[F]reeBSD.*?\$)/);
			$version = $1;
			last;
		}
		close(FILE);
	}
	sysopen(FILE, $service, O_RDWR|O_CREAT|O_TRUNC)
	    or die("$service: $!\n");
	print(FILE "#\n");
	print(FILE "# $version\n");
	print(FILE "#\n");
	print(FILE "# PAM configuration for the \"$service\" service\n");
	print(FILE "#\n");
	foreach $type (qw(auth account session password)) {
	    next unless exists($SERVICES{$service}->{$type});
	    print(FILE "\n");
	    print(FILE "# $type\n");
	    print(FILE join("\n", @{$SERVICES{$service}->{$type}}, ""));
	}
	close(FILE);
	warn("$service\n");
    }

    exit(0);
}