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

#
#  Convert GECOS information in password files to alias syntax.
#
#  Contributed by Kari E. Hurtta <Kari.Hurtta@ozone.fmi.fi>
#

print "# Generated from passwd by $0\n";

$wordpat = '([a-zA-Z]+?[a-zA-Z0-9-]*)?[a-zA-Z0-9]';	# 'DB2'
while (@a = getpwent) {
    ($name,$passwd,$uid,$gid,$quota,$comment,$gcos,$dir,$shell) = @a;

    ($fullname = $gcos) =~ s/,.*$//;

    if (!-d $dir || !-x $shell || $shell =~ m!/bin/(false|true)$!) {
	print "$name: root\n";				# handle pseudo user
    }

    $fullname =~ s/\.*[ _]+\.*/./g;
    $fullname =~ tr [Â‰Èˆ¸≈ƒ÷‹] [aaeouAAOU];  # <hakan@af.lu.se> 1997-06-15
    next if (!$fullname || lc($fullname) eq $name);	# avoid nonsense
    if ($fullname =~ /^$wordpat(\.$wordpat)*$/o) {	# Ulrich Windl
	print "$fullname: $name\n";
    } else {
	print "# $fullname: $name\n";			# avoid strange names
    }
};

endpwent;