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
# Regression test for GNU grep.
: ${srcdir=.}
: ${GREP=../src/grep}

failures=0

# The Khadafy test is brought to you by Scott Anderson . . .

${GREP} -E -f $srcdir/khadafy.regexp $srcdir/khadafy.lines > khadafy.out
if cmp $srcdir/khadafy.lines khadafy.out
then
	:
else
	echo Khadafy test failed -- output left on khadafy.out
	failures=1
fi

exit $failures