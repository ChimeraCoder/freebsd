
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
# $FreeBSD$
echo 1..14

REGRESSION_START($1)

REGRESSION_TEST(`00', `tr abcde 12345 < regress.in')
REGRESSION_TEST(`01', `tr 12345 abcde < regress.in')
REGRESSION_TEST(`02', `tr -d aceg < regress.in')
REGRESSION_TEST(`03', `tr "[[:lower:]]" "[[:upper:]]" < regress.in')
REGRESSION_TEST(`04', `tr "[[:alpha:]]" . < regress.in')
REGRESSION_TEST(`05', `tr "[[:lower:]]" "[[:upper:]]" < regress.in | tr "[[:upper:]]" "[[:lower:]]"')
REGRESSION_TEST(`06', `tr "[[:digit:]]" "?" < regress2.in')
REGRESSION_TEST(`07', `tr "[[:alnum:]]" "#" < regress2.in')
REGRESSION_TEST(`08', `tr "[[:upper:]]" "[[:lower:]]" < regress2.in | tr -d "[^[:alpha:]] "')
REGRESSION_TEST(`09', `printf "\\f\\r\\n" | tr "\\014\\r" "?#"')
REGRESSION_TEST(`0a', `printf "0xdeadbeef\\n" | tr "x[[:xdigit:]]" "?\$"')
REGRESSION_TEST(`0b', `(tr -cd "[[:xdigit:]]" < regress2.in ; echo)')
REGRESSION_TEST(`0c', `echo "[[[[]]]]" | tr -d "[=]=]"')
REGRESSION_TEST(`0d', `echo "]=[" | tr -d "[=]"')

REGRESSION_END()