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
# used to generate the file MINFO for use by util/mk1mf.pl
# It is basically a list of all variables from the passed makefile
#
$s="";
while (<>)
	{
	chop;
	s/#.*//;
	if (/^(\S+)\s*=\s*(.*)$/)
		{
		$o="";
		($s,$b)=($1,$2);
		for (;;)
			{
			if ($b =~ /\\$/)
				{
				chop($b);
				$o.=$b." ";
				$b=<>;
				chop($b);
				}
			else
				{
				$o.=$b." ";
				last;
				}
			}
		$o =~ s/^\s+//;
		$o =~ s/\s+$//;
		$o =~ s/\s+/ /g;

		$o =~ s/\$[({]([^)}]+)[)}]/$sym{$1}/g;
		$sym{$s}=$o;
		}
	}

$pwd=`pwd`; chop($pwd);

if ($sym{'TOP'} eq ".")
	{
	$n=0;
	$dir=".";
	}
else	{
	$n=split(/\//,$sym{'TOP'});
	@_=split(/\//,$pwd);
	$z=$#_-$n+1;
	foreach $i ($z .. $#_) { $dir.=$_[$i]."/"; }
	chop($dir);
	}

print "RELATIVE_DIRECTORY=$dir\n";

foreach (sort keys %sym)
	{
	print "$_=$sym{$_}\n";
	}
print "RELATIVE_DIRECTORY=\n";