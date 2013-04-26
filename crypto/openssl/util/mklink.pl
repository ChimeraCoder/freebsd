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

# mklink.pl

# The first command line argument is a non-empty relative path
# specifying the "from" directory.
# Each other argument is a file name not containing / and
# names a file in the current directory.
#
# For each of these files, we create in the "from" directory a link
# of the same name pointing to the local file.
#
# We assume that the directory structure is a tree, i.e. that it does
# not contain symbolic links and that the parent of / is never referenced.
# Apart from this, this script should be able to handle even the most
# pathological cases.

use Cwd;

my $from = shift;
my @files = @ARGV;

my @from_path = split(/[\\\/]/, $from);
my $pwd = getcwd();
chomp($pwd);
my @pwd_path = split(/[\\\/]/, $pwd);

my @to_path = ();

my $dirname;
foreach $dirname (@from_path) {

    # In this loop, @to_path always is a relative path from
    # @pwd_path (interpreted is an absolute path) to the original pwd.

    # At the end, @from_path (as a relative path from the original pwd)
    # designates the same directory as the absolute path @pwd_path,
    # which means that @to_path then is a path from there to the original pwd.

    next if ($dirname eq "" || $dirname eq ".");

    if ($dirname eq "..") {
	@to_path = (pop(@pwd_path), @to_path);
    } else {
	@to_path = ("..", @to_path);
	push(@pwd_path, $dirname);
    }
}

my $to = join('/', @to_path);

my $file;
$symlink_exists=eval {symlink("",""); 1};
if ($^O eq "msys") { $symlink_exists=0 };
foreach $file (@files) {
    my $err = "";
    if ($symlink_exists) {
	unlink "$from/$file";
	symlink("$to/$file", "$from/$file") or $err = " [$!]";
    } else {
	unlink "$from/$file"; 
	open (OLD, "<$file") or die "Can't open $file: $!";
	open (NEW, ">$from/$file") or die "Can't open $from/$file: $!";
	binmode(OLD);
	binmode(NEW);
	while (<OLD>) {
	    print NEW $_;
	}
	close (OLD) or die "Can't close $file: $!";
	close (NEW) or die "Can't close $from/$file: $!";
    }
    print $file . " => $from/$file$err\n";
}