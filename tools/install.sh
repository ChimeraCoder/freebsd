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

# parse install's options and ignore them completely.
dirmode=""
linkmode=""
while [ $# -gt 0 ]; do
    case $1 in
    -d) dirmode="YES"; shift;;
    -[bCcpSsv]) shift;;
    -[BDfghMmNoTU]) shift; shift;;
    -[BDfghMmNoTU]*) shift;;
    -l)
	shift
	case $1 in
	*[sm]*) linkmode="symbolic";;	# XXX: 'm' should prefer hard
	*h*) linkmode="hard";;
	*) echo "invalid link mode"; exit 1;;
	esac
	shift
	;;
    *) break;
    esac
done

if [ "$#" -eq 0 ]; then
	echo "$0: no files/dirs specified" >&2
	exit 1
fi

if [ -z "$dirmode" ] && [ "$#" -lt 2 ]; then
	echo "$0: no target specified" >&2
	exit 1
fi

# the remaining arguments are assumed to be files/dirs only.
if [ -n "${linkmode}" ]; then
	if [ "${linkmode}" = "symbolic" ]; then
		ln -fsh "$@"
	else
		ln -f "$@"
	fi
elif [ -z "$dirmode" ]; then
	exec install -p "$@"
else
	exec install -d "$@"
fi