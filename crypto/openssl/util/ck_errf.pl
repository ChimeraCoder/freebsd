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
# This is just a quick script to scan for cases where the 'error'
# function name in a XXXerr() macro is wrong.
# 
# Run in the top level by going
# perl util/ck_errf.pl */*.c */*/*.c
#
my $err_strict = 0;
my $bad = 0;

foreach $file (@ARGV)
	{
	if ($file eq "-strict")
		{
		$err_strict = 1;
		next;
		}
	open(IN,"<$file") || die "unable to open $file\n";
	$func="";
	while (<IN>)
		{
		if (!/;$/ && /^([a-zA-Z].*[\s*])?([A-Za-z_0-9]+)\(.*[),]/)
			{
			/^([^()]*(\([^()]*\)[^()]*)*)\(/;
			$1 =~ /([A-Za-z_0-9]*)$/;
			$func = $1;
			$func =~ tr/A-Z/a-z/;
			}
		if (/([A-Z0-9]+)err\(([^,]+)/ && ! /ckerr_ignore/)
			{
			$errlib=$1;
			$n=$2;

			if ($func eq "")
				{ print "$file:$.:???:$n\n"; $bad = 1; next; }

			if ($n !~ /([^_]+)_F_(.+)$/)
				{
		#		print "check -$file:$.:$func:$n\n";
				next;
				}
			$lib=$1;
			$n=$2;

			if ($lib ne $errlib)
				{ print "$file:$.:$func:$n [${errlib}err]\n"; $bad = 1; next; }

			$n =~ tr/A-Z/a-z/;
			if (($n ne $func) && ($errlib ne "SYS"))
				{ print "$file:$.:$func:$n\n"; $bad = 1; next; }
	#		print "$func:$1\n";
			}
		}
	close(IN);
        }

if ($bad && $err_strict)
	{
	print STDERR "FATAL: error discrepancy\n";
	exit 1;
	}