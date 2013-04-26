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
# modify the '#!/usr/local/bin/perl'
# line in all scripts that rely on perl.
#
require "find.pl";

$#ARGV == 0 || print STDERR "usage: perlpath newpath  (eg /usr/bin)\n";
&find(".");

sub wanted
	{
	return unless /\.pl$/ || /^[Cc]onfigur/;

	open(IN,"<$_") || die "unable to open $dir/$_:$!\n";
	@a=<IN>;
	close(IN);

	if (-d $ARGV[0]) {
		$a[0]="#!$ARGV[0]/perl\n";
	}
	else {
		$a[0]="#!$ARGV[0]\n";
	}

	# Playing it safe...
	$new="$_.new";
	open(OUT,">$new") || die "unable to open $dir/$new:$!\n";
	print OUT @a;
	close(OUT);

	rename($new,$_) || die "unable to rename $dir/$new:$!\n";
	chmod(0755,$_) || die "unable to chmod $dir/$new:$!\n";
	}