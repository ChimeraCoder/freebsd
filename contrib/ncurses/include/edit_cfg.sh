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
BAK=save$$
TMP=edit$$
trap "rm -f $BAK $TMP" 0 1 2 5 15
for name in \
	HAVE_TCGETATTR \
	HAVE_TERMIOS_H \
	HAVE_TERMIO_H \
	BROKEN_LINKER
do
	mv $2 $BAK
	if ( grep "[ 	]$name[ 	]" $1 2>&1 >$TMP )
	then
		value=1
	else
		value=0
	fi
	echo '** edit: '$name $value
	sed \
		-e "s@#define ${name}.*\$@#define $name $value@" \
		-e "s@#if $name\$@#if $value /* $name */@" \
		-e "s@#if !$name\$@#if $value /* !$name */@" \
		$BAK >$2
	if (cmp -s $2 $BAK)
	then
		mv $BAK $2
	else
		rm -f $BAK
	fi
done