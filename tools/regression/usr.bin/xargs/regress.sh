
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
echo 1..5

REGRESSION_START($1)

REGRESSION_TEST(`normal', `xargs echo The < regress.in')
REGRESSION_TEST(`I', `xargs -I% echo The % % % %% % % < regress.in')
REGRESSION_TEST(`J', `xargs -J% echo The % again. < regress.in')
REGRESSION_TEST(`L', `xargs -L3 echo < regress.in')
REGRESSION_TEST(`R', `xargs -I% -R1 echo The % % % %% % % < regress.in')

REGRESSION_END()