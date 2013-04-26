
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
echo 1..9

REGRESSION_START($1)

REGRESSION_TEST(`1', `head -c 13 regress.in | file2c')
REGRESSION_TEST(`2', `head -c 26 regress.in | file2c PREFIX')
REGRESSION_TEST(`3', `head -c 39 regress.in | file2c PREFIX SUFFIX')
REGRESSION_TEST(`4', `head -c 52 regress.in | file2c -x')
REGRESSION_TEST(`5', `head -c 65 regress.in | file2c -n -1')

REGRESSION_TEST(`6', `head -c  7 regress.in | file2c -n 1 P S')
REGRESSION_TEST(`7', `head -c 14 regress.in | file2c -n 2 -x "P S"')
REGRESSION_TEST(`8', `head -c 21 regress.in | file2c -n 16 P -x S')

REGRESSION_TEST(`9', `file2c "const char data[] = {" ", 0};" < regress.in')

REGRESSION_END()