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

NANO_PKG_DUMP=$1
shift;
if [ ! -d $NANO_PKG_DUMP ] ; then
	echo "$NANO_PKG_DUMP not a directory" 1>&2
	exit 1
fi

NANO_PACKAGE_DIR=$1
shift;

ports_recurse() (
	of=$1
	shift
	for d
	do
		if [ ! -d $d ] ; then
			echo "Missing port $d" 1>&2
			exit 2
		fi
		if grep -q "^$d\$" $of ; then
			true
		else
			(
			cd $d
			rd=`make -V RUN_DEPENDS`	
			ld=`make -V LIB_DEPENDS`	
			
			for x in $rd $ld
			do
				ports_recurse $of `echo $x |
				    sed 's/^[^:]*:\([^:]*\).*$/\1/'`
			done
			)
			echo $d >> $of
		fi
	done
)

rm -rf $NANO_PACKAGE_DIR
mkdir -p $NANO_PACKAGE_DIR

PL=$NANO_PACKAGE_DIR/_list
true > $PL
for i 
do
	ports_recurse `pwd`/$PL $i
done

for i in `cat $PL`
do
	p=`(cd $i && make -V PKGNAME)`
	if [ -f $NANO_PKG_DUMP/$p.tbz ] ; then
		ln -s $NANO_PKG_DUMP/$p.tbz $NANO_PACKAGE_DIR
	else
		echo "Package $p misssing in $NANO_PKG_DUMP" 1>&2
		exit 1
	fi
done

rm -f $PL
exit 0