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

PATH=/bin:/usr/bin:$PATH; export PATH
opt= dirs= localdirs=

for arg
do
	case "$arg" in
		-*) 	opt="$opt $arg";;
		*)	dirs="$dirs $arg";;
	esac
done

dirs=`echo $dirs | sed 's/:/ /g'`
case X"$dirs" in X) echo "usage: $0 [options] directories ..."; exit 1;; esac

localdirs=`find -H $dirs -fstype local -type d -prune -print`

case X"$localdirs" in
	X) 	echo "$0: no local-mounted manual directories found: $dirs"
		exit 1;;
	*) 	exec `basename $0 .local` $opt $localdirs;;
esac