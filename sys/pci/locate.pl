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
# $FreeBSD$
use strict;

if (!defined($ARGV[0])) {
    print(
"
Perl script to convert NCR script address into label+offset.
Useful to find the failed NCR instruction ...

usage: $0 <address>
");
    exit(1);
}

my $errpos = hex($ARGV[0])/4;
my $ofs=0;

open (INPUT, "cc -E ncr.c 2>/dev/null |");

while ($_ = <INPUT>)
{
    last if /^struct script \{/;
}

while ($_ = <INPUT>)
{
    last if /^\}\;/;
    my ($label, $size) = /ncrcmd\s+(\S+)\s+\[([^]]+)/;
    $size = eval($size);
    if (defined($label) && $label) {
	if ($errpos) {
	    if ($ofs + $size > $errpos) {
		printf ("%4x: %s\n", $ofs * 4, $label);
		printf ("%4x: %s + %d\n", $errpos * 4, $label, $errpos - $ofs);
		last;
	    }
	    $ofs += $size;
	} else {
	    printf ("%4x: %s\n", $ofs * 4, $label);
	}
    }
}