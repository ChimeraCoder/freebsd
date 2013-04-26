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
# Validate all locales installed in specified directory
# (by default check system locales)
#
# $FreeBSD$
#
LOCALEDIR=/usr/share/locale

if [ "$1" != "" ]; then
	LOCALEDIR=$1
fi

if [ ! -x ./localeck ]; then
	echo "ERROR: build test program first."
	exit 1
fi

PATH_LOCALE=$LOCALEDIR
LOCALES=0
ERRORS=0

echo "Validating locales in $LOCALEDIR"
echo

for i in `ls -1 $LOCALEDIR`
do
	LOCALES=$(($LOCALES + 1))
	./localeck $i || ERRORS=$(($ERRORS + 1))
done

echo
echo "Validation test complete"
echo "$LOCALES locales were checked"
echo "$ERRORS invalid locales were found"