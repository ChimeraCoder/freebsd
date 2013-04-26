
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
LC_ALL=C; export LC_ALL

echo 1..3

REGRESSION_START($1)

REGRESSION_TEST(`00', `comm -12 regress.00a.in regress.00b.in')
REGRESSION_TEST(`01', `comm -12 regress.01a.in regress.01b.in')
REGRESSION_TEST(`02', `comm regress.02a.in regress.02b.in')

REGRESSION_END()