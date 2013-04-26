#!/bin/sh
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
#	@(#)makedb.sh	8.1 (Berkeley) 6/4/93
awk '{i++; print $0; print i;}' /usr/share/dict/words > WORDS
ls /bin /usr/bin /usr/ucb /etc | egrep '^(...|....|.....|......)$' | \
sort | uniq | \
awk '{
	printf "%s\n", $0
	for (i = 0; i < 1000; i++)
		printf "%s+", $0
	printf "\n"
}' > LONG.DATA