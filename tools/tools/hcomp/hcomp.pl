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

use strict;
use Getopt::Std;

our $opt_b;
our $opt_v;

sub hcomp($)
{
    my $fn = shift;

    local *FILE;
    my $header;

    warn("$fn\n")
	if ($opt_v);

    open(FILE, "<", $fn)
	or die("$fn: $!\n");
    $header = join('', <FILE>);
    close(FILE);

    # Remove comments
    $header =~ s|/\*.*?\*/||gs;
    $header =~ s|//.*$||gm;

    # Collapse preprocessor directives
    while ($header =~ s|(\n\#.*?)\\\n|$1|gs) {
	# nothing
    }

    # Remove superfluous whitespace
    $header =~ s|^\s+||s;
    $header =~ s|^\s+||gm;
    $header =~ s|\s+$||s;
    $header =~ s|\s+$||gm;
    $header =~ s|\n+|\n|gs;
    $header =~ s|[ \t]+| |gm;

    open(FILE, ">", "$fn.new")
	or die("$fn.new: $!\n");
    print(FILE $header);
    close(FILE);

    rename($fn, "$fn.$opt_b")
	if defined($opt_b);
    rename("$fn.new", $fn);
}

sub usage()
{
    print(STDERR "usage: hcomp [-b ext] file ...\n");
    exit(1);
}

MAIN:{
    my %opts;
    getopts('b:v')
	or usage();
    foreach (@ARGV) {
	hcomp($_);
    }
}