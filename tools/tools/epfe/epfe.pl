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

$in = 0; @a = ();
sub Print { s/\&amp\;/&/g; push(@a,$_); }
sub out { 
    local($name, *lines) = @_;
    open(F, "> $name") || die "open $_[0]: $!\n"; 
    print F @lines;
    close F;
}

while(<>) {
    if (/^<code>/) {
	$in = 1;
    } elsif (m%</code>% && $in > 0) {
	if ($in > 1) {
	    $name = 'unknown' if !$name;
	    while(1) { if ($d{$name}) { $name .= 'X'; } else { last } }
	    &out("$name", *a);
	    $d{$name} = $name;
	}
	$in = 0; $name = ''; @a = ();
    } elsif ($in == 1 && /^\#\s*!/) {
	$in++; &Print;
    } elsif ($in > 1) {
	$name = $1 if (!$name && /^\#\s+(\S+)\s+-\s+/);
	$in++; &Print;
    }
}