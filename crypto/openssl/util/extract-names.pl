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

$/ = "";			# Eat a paragraph at once.
while(<STDIN>) {
    chop;
    s/\n/ /gm;
    if (/^=head1 /) {
	$name = 0;
    } elsif ($name) {
	if (/ - /) {
	    s/ - .*//;
	    s/,\s+/,/g;
	    s/\s+,/,/g;
	    s/^\s+//g;
	    s/\s+$//g;
	    s/\s/_/g;
	    push @words, split ',';
	}
    }
    if (/^=head1 *NAME *$/) {
	$name = 1;
    }
}

print join("\n", @words),"\n";