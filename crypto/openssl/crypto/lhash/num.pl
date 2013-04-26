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

#node     10 ->   4

while (<>)
	{
	next unless /^node/;
	chop;
	@a=split;
	$num{$a[3]}++;
	}

@a=sort {$a <=> $b } keys %num;
foreach (0 .. $a[$#a])
	{
	printf "%4d:%4d\n",$_,$num{$_};
	}