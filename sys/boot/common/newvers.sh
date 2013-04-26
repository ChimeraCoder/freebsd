#!/bin/sh -
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

LC_ALL=C; export LC_ALL
u=${USER-root} h=${HOSTNAME-`hostname`} t=`date`
#r=`head -n 6 $1 | tail -n 1 | awk -F: ' { print $1 } '`
r=`awk -F: ' /^[0-9]\.[0-9]+:/ { print $1; exit }' $1`

echo "char bootprog_name[] = \"FreeBSD/${3} ${2}\";" > vers.c
echo "char bootprog_rev[] = \"${r}\";" >> vers.c
echo "char bootprog_date[] = \"${t}\";" >> vers.c
echo "char bootprog_maker[] = \"${u}@${h}\";" >> vers.c