
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
# $FreeBSD$sysctl debug.mutex.prof.stats | awk '$1 ~ /[0-9]+/ { if ($3 != 0) { hld_prc = $5 / $3 * 100; lck_prc = $6 / $3 * 100 } else { hld_prc = 0; lck_prc = 0 } print $1 " " $2 " " $3 " " $4 " " $5 " " hld_prc " " $6 " " lck_prc " " substr($0, index($0, $7)); next } { print }'