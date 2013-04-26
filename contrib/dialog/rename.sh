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

LANG=C;     export LANG
LC_ALL=C;   export LC_ALL
LC_CTYPE=C; export LC_CTYPE
LANGUAGE=C; export LANGUAGE

SOURCE=$1; shift
TARGET=$1; shift
BINARY=$1; shift
PACKAGE=$1; shift

CHR_LEAD=`echo "$BINARY" | sed -e 's/^\(.\).*/\1/'`
CHR_TAIL=`echo "$BINARY" | sed -e 's/^.//'`
ONE_CAPS=`echo $CHR_LEAD | tr '[a-z]' '[A-Z]'`$CHR_TAIL
ALL_CAPS=`echo "$BINARY" | tr '[a-z]' '[A-Z]'`

sed	-e "s,^\.ds p dialog\>,.ds p $BINARY," \
	-e "s,^\.ds l dialog\>,.ds l $PACKAGE," \
	-e "s,^\.ds L Dialog\>,.ds L $ONE_CAPS," \
	-e "s,^\.ds D DIALOG\>,.ds D $ALL_CAPS," \
	<$SOURCE >source.tmp
"$@" source.tmp $TARGET
rm -f source.tmp