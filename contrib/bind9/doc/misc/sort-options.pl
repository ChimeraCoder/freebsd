#!/bin/perl
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

# $Id: sort-options.pl,v 1.3 2007/09/24 23:46:48 tbox Exp $

sub sortlevel() {
	my @options = ();
	my $fin = "";
	my $i = 0;
	while (<>) {
		if (/^\s*};$/) {
			$fin = $_;
			# print 2, $_;
			last;
		}
		next if (/^$/);
		if (/{$/) {
			# print 3, $_;
			my $sec = $_;
			push(@options, $sec . sortlevel());
		} else {
			push(@options, $_);
			# print 1, $_;
		}
		$i++;
	}
	my $result = "";
	foreach my $i (sort @options) {
		$result = ${result}.${i};
		$result = $result."\n" if ($i =~ /^[a-z]/i);
		# print 5, ${i};
	}
	$result = ${result}.${fin};
	return ($result);
}

print sortlevel();