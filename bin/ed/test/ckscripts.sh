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
# This script runs the .ed scripts generated by mkscripts.sh
# and compares their output against the .r files, which contain
# the correct output
#
# $FreeBSD$
PATH="/bin:/usr/bin:/usr/local/bin/:."
ED=$1
[ ! -x $ED ] && { echo "$ED: cannot execute"; exit 1; }

# Run the *.red scripts first, since these don't generate output;
# they exit with non-zero status
for i in *.red; do
	echo $i
	if $i; then
		echo "*** The script $i exited abnormally  ***"
	fi
done >errs.o 2>&1

# Run the remainding scripts; they exit with zero status
for i in *.ed; do
#	base=`expr $i : '\([^.]*\)'`
#	base=`echo $i | sed 's/\..*//'`
	base=`$ED - \!"echo $i" <<-EOF
		s/\..*
	EOF`
	if $base.ed; then
		if cmp -s $base.o $base.r; then :; else
			echo "*** Output $base.o of script $i is incorrect ***"
		fi
	else
		echo "*** The script $i exited abnormally ***"
	fi
done >scripts.o 2>&1

grep -h '\*\*\*' errs.o scripts.o