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

voptions=""
options=""
files=""
f=""
head=""
vf="/usr/libexec/vfontedpr"
tm="/usr/share/tmac"
postproc="psroff"

# Parse args
while test $# -gt 0; do
	case $1 in
	-f)
		f="filter"
		options="$options -f"
	;;
	-t)
		voptions="$voptions -t"
	;;
	-o*)
		voptions="$voptions $1"
	;;
	-W)
		voptions="$voptions -W"
	;;
	-d)
		if test $# -lt 2; then
			echo "$0: option $1 must have argument" >&2
			exit 1
		fi
		options="$options $1 $2"
		shift
	;;
	-h)
		if test $# -lt 2; then
			echo "$0: option $1 must have argument" >&2
			exit 1
		fi
		head="$2"
		shift
	;;
	-p)
		if test $# -lt 2; then
			echo "$0: option $1 must have argument" >&2
			exit 1
		fi
		postproc="$2"
		shift
	;;
	-*)
		options="$options $1"
	;;
	*)
		files="$files $1"
	;;
	esac
	shift
done

if test -r index; then
	echo > nindex
	for i in $files; do
		#       make up a sed delete command for filenames
		#       being careful about slashes.
		echo "? $i ?d" | sed -e "s:/:\\/:g" -e "s:?:/:g" >> nindex
	done
	sed -f nindex index > xindex
	if test "x$f" = xfilter; then
		if test "x$head" != x; then
			$vf $options -h "$head" $files
		else
			$vf $options $files
		fi | cat $tm/tmac.vgrind -
	else
		if test "x$head" != x; then
			$vf $options -h "$head" $files
		else
			$vf $options $files
		fi | sh -c "$postproc -rx1 $voptions -i -mvgrind 2>> xindex"
	fi
	sort -df -k 1,2 xindex > index
	rm nindex xindex
else
	if test "x$f" = xfilter; then
		if test "x$head" != x; then
			$vf $options -h "$head" $files
		else
			$vf $options $files
		fi | cat $tm/tmac.vgrind -
	else
		if test "x$head" != x; then
			$vf $options -h "$head" $files
		else
			$vf $options $files
		fi | $postproc -i $voptions -mvgrind
	fi
fi