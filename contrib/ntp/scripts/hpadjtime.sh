#! /bin/sh
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
val=1
if [ -f /bin/uname -o -f /usr/bin/uname ]; then
	set `uname -a | tr '[A-Z]' '[a-z]'`
	case "$1" in
		hp-ux)  case "$3" in
			*.10.*) val=1 ;;
			*.09.03 | *.09.10) case "$5" in
				9000/3*) val=1 ;;
				*)       val=0 ;;
				esac ;;
			*) val=0 ;;
			esac
			;;
	*)
	esac
fi
exit $val