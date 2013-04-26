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
#
# This generates a version string which includes recent version as
# specified in correspondent sub project's configure.in file, plus
# git revision abbreviation in the case if sub-project HEAD is different
# from recent tag, plus "-dirty" suffix if local uncommitted changes are
# in the sub project tree.
#
usage()
{
	echo "Usage: $0 <target>"
	exit 2
}

test -z "$1" && usage

package=$1

cd `dirname $0`

conf_file=$package/configure.in
version=`cat $conf_file | sed -ne '/AC_INIT.*'$package'.*/s/^AC_INIT.*'$package', \(.*\),.*$/\1/p'`

git diff --quiet $package-$version..HEAD -- $package > /dev/null 2>&1
if [ $? -eq 1 ] ; then
	abbr=`git rev-parse --short --verify HEAD 2>/dev/null`
	if [ ! -z "$abbr" ] ; then
		version="${version}_${abbr}"
	fi
fi

git diff-index --quiet HEAD -- $package > /dev/null 2>&1
if [ $? -eq 1 ] ; then
	version="${version}_dirty"
fi

echo $version