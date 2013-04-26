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
DOIT=eval

if test $# != 3 ; then
	echo "Usage: $0 files source target"
	exit 1
elif test ! -d "$2" ; then
	echo "Source directory not found: $2"
	exit 1
elif test ! -d "$3" ; then
	echo "Target directory not found: $3"
	exit 1
fi

WD=`pwd`

TMP=$WD/copy$$

cd $2
TEST=`ls -d $1 2>/dev/null`
if test -z "$TEST"
then
	echo "... no match for \"$1\" in $2"
else
	echo "... installing files matching \"$1\" in $2"
	trap "rm -f $TMP" 0 1 2 5 15
	if ( tar cf $TMP $1 )
	then
		cd $3
		LIST=`tar tf $TMP 2>&1`
		$DOIT rm -rf $LIST 2>/dev/null
		$DOIT tar xvf $TMP
	else
		echo "Cannot create tar of $1 files"
		exit 1
	fi
fi