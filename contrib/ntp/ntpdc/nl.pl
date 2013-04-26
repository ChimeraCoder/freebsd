#! /usr/local/bin/perl -w
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

$found = 0;
$last = 0;
$debug = 0;

while (<>) {
    next if /^#/;
    next if /^\s*$/;
    if (/^struct req_pkt/) {
	$found = 1;
    }
    if (/^struct info_dns_assoc/) {
	$last = 1;
    }
    if ($found) {
	if (/^(struct\s*\w*)\s*{\s*$/) {
	    $type = $1;
	    print STDERR "type = '$type'\n" if $debug;
	    printf "  printf(\"sizeof($type) = %%d\\n\", \n\t (int) sizeof($type));\n";
	    next;
	}
	if (/\s*\w+\s+(\w*)\s*(\[.*\])?\s*;\s*$/) {
	    $field = $1;
	    print STDERR "\tfield = '$field'\n" if $debug;
	    printf "  printf(\"offsetof($field) = %%d\\n\", \n\t (int) offsetof($type, $field));\n";
	    next;
	}
	if (/^}\s*;\s*$/) {
	    printf "  printf(\"\\n\");\n\n";
	    $found = 0 if $last;
	    next;
	}
	print STDERR "Unmatched line: $_";
	exit 1;
    }
}