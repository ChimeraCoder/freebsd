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

print "1..30\n";

$i = 0;
# Iterates 10 times.
for ($stacksize = 65536; $stacksize < 131072; $stacksize += 7168)
{
    # Iterates 3 times (1024, 4096, 7168).
    for ($guardsize = 1024; $guardsize < 8192; $guardsize += 3072)
    {
	$i++;

	print "stacksize: $stacksize, guardsize: $guardsize\n";

	`./guard_b $stacksize $guardsize >guard_b.out 2>&1`;

	if (! -f "./guard_b.out")
	{
	    print "not ok $i\n";
	}
	else
	{
	    `diff guard_b.exp guard_b.out >guard_b.diff 2>&1`;
	    if ($?)
	    {
		# diff returns non-zero if there is a difference.
		print "not ok $i\n";
	    }
	    else
	    {
		print "ok $i\n";
	    }
	}
    }
}