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
# $FreeBSD$
#
# For each sysctl, repeat:
#	if it has a short description
#		sysctl.sh name "descr"
#	else
#		write its name to tunables.TODO with 'name missing description'
#		Note: This functionality is to point out which sysctls/tunables
#		have no description in the source.  This may be helpful for those
#		wishing to document the sysctls.
#
name="$1"
if [ X"${name}" = X"" ]; then
	echo "usage: $(basename $0) sysctl-name" >&2
	exit 1
fi


# Look up $name in tunables.mdoc

< tunables.mdoc \
sed -ne "/^${name}[[:space:]]*$/,/^---[[:space:]]*$/p" |	\
sed -e '/^---[[:space:]]*$/d' |					\

{								\
	read tmpname _junk;					\
	if [ X"${tmpname}" = X"" ]; then			\
		exit 0;						\
	fi ;							\
	read type value _junk;					\
	unset _junk;						\
	if [ X"${type}" = X"" ]; then				\
		echo "" >&2 ;					\
		echo "ERROR: Missing type for ${name}" >&2 ;	\
	fi ;							\
	if [ X"${value}" = X"" ]; then				\
		echo "" >&2 ;					\
		echo "ERROR: Missing default for ${name}" >&2 ;	\
	fi ;							\

	echo ".It Va ${tmpname}" ;				\
    	if [ X"${type}" != X"" ]; then				\
		echo ".Pq Vt ${type}" ;				\
	fi ;							\
	grep -v '^[[:space:]]*$' |				\
	sed -e "s/@default@/${value}/g" |			\
	sed -e "s/@type@/${type}/g" ;				\
}