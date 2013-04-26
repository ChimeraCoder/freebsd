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
# $FreeBSD$
run() { ../shlib-compat.py --no-dump -vv libtest$1/libtest$1.so.0.debug libtest$2/libtest$2.so.0.debug; }
echo 1..9
REGRESSION_START($1)
REGRESSION_TEST(`1-1', `run 1 1')
REGRESSION_TEST(`1-2', `run 1 2')
REGRESSION_TEST(`1-3', `run 1 3')
REGRESSION_TEST(`2-1', `run 2 1')
REGRESSION_TEST(`2-2', `run 2 2')
REGRESSION_TEST(`2-3', `run 2 3')
REGRESSION_TEST(`3-1', `run 3 1')
REGRESSION_TEST(`3-2', `run 3 2')
REGRESSION_TEST(`3-3', `run 3 3')
REGRESSION_END()