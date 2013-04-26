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

ncopies=""
symlink="-s"
mailafter=""
title=""

# Posix says LPDEST gets precedence over PRINTER
dest=${LPDEST:-${PRINTER:-lp}}

#
# XXX We include the -o flag as a dummy.  Posix 1003.2 does not require
# it, but the rationale mentions it as a possible future extension.
# XXX We include the -s flag as a dummy.  SUSv2 requires it,
# although we do not yet emit the affected messages.
#
while getopts "cd:mn:o:st:" option
do
	case $option in

	c)			# copy files before printing
		symlink="";;
	d)			# destination
		dest="${OPTARG}";;
	m)			# mail after job
		mailafter="-m";;
	n)			# number of copies
		ncopies="-#${OPTARG}";;
	o)			# (printer option)
		: ;;
	s)			# (silent option)
		: ;;
	t)			# title for banner page
		title="${OPTARG}";;
	*)			# (error msg printed by getopts)
		exit 2;;
	esac
done

shift $(($OPTIND - 1))

exec /usr/bin/lpr "-P${dest}" ${symlink} ${ncopies} ${mailafter} ${title:+-J"${title}"} "$@"