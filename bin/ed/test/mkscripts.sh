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
# This script generates ed test scripts (.ed) from .t files
#	
# $FreeBSD$
PATH="/bin:/usr/bin:/usr/local/bin/:."
ED=$1
[ ! -x $ED ] && { echo "$ED: cannot execute"; exit 1; }

for i in *.t; do
#	base=${i%.*}
#	base=`echo $i | sed 's/\..*//'`
#	base=`expr $i : '\([^.]*\)'`
#	(
#	echo "#!/bin/sh -"
#	echo "$ED - <<\EOT"
#	echo "r $base.d"
#	cat $i
#	echo "w $base.o"
#	echo EOT
#	) >$base.ed
#	chmod +x $base.ed
# The following is pretty ugly way of doing the above, and not appropriate 
# use of ed  but the point is that it can be done...
	base=`$ED - \!"echo $i" <<-EOF
		s/\..*
	EOF`
	$ED - <<-EOF
		a
		#!/bin/sh -
		$ED - <<\EOT
		H
		r $base.d
		w $base.o
		EOT
		.
		-2r $i
		w $base.ed
		!chmod +x $base.ed
	EOF
done

for i in *.err; do
#	base=${i%.*}
#	base=`echo $i | sed 's/\..*//'`
#	base=`expr $i : '\([^.]*\)'`
#	(
#	echo "#!/bin/sh -"
#	echo "$ED - <<\EOT"
#	echo H
#	echo "r $base.err"
#	cat $i
#	echo "w $base.o"
#	echo EOT
#	) >$base-err.ed
#	chmod +x $base-err.ed
# The following is pretty ugly way of doing the above, and not appropriate 
# use of ed  but the point is that it can be done...
	base=`$ED - \!"echo $i" <<-EOF
		s/\..*
	EOF`
	$ED - <<-EOF
		a
		#!/bin/sh -
		$ED - <<\EOT
		H
		r $base.err
		w $base.o
		EOT
		.
		-2r $i
		w ${base}.red
		!chmod +x ${base}.red
	EOF
done