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

# mkdir-p.pl

# On some systems, the -p option to mkdir (= also create any missing parent
# directories) is not available.

my $arg;

foreach $arg (@ARGV) {
  $arg =~ tr|\\|/|;
  &do_mkdir_p($arg);
}


sub do_mkdir_p {
  local($dir) = @_;

  $dir =~ s|/*\Z(?!\n)||s;

  if (-d $dir) {
    return;
  }

  if ($dir =~ m|[^/]/|s) {
    local($parent) = $dir;
    $parent =~ s|[^/]*\Z(?!\n)||s;

    do_mkdir_p($parent);
  }

  mkdir($dir, 0777) || die "Cannot create directory $dir: $!\n";
  print "created directory `$dir'\n";
}