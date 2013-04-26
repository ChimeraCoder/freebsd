#!/usr/local/bin/perl
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
# tack error codes onto the end of a file
#
open(ERR,$ARGV[0]) || die "unable to open error file '$ARGV[0]':$!\n";
@err=<ERR>;
close(ERR);

open(IN,$ARGV[1]) || die "unable to open header file '$ARGV[1]':$!\n";

@out="";
while (<IN>)
	{
	push(@out,$_);
	last if /BEGIN ERROR CODES/;
	}
close(IN);

open(OUT,">$ARGV[1]") || die "unable to open header file '$ARGV[1]':$1\n";
print OUT @out;
print OUT @err;
print OUT <<"EOF";
 
#ifdef  __cplusplus
}
#endif
#endif

EOF
close(OUT);