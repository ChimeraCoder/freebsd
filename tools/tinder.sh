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
# Utility script to build specific parts of the source tree on all arches
#
# Example:
#
# cd /usr/src
# make toolchains		# build toolchain for all arches
# sh tools/tinder.sh gnu/lib/libdialog usr.sbin/sade NO_CLEAN=yes
#				# build libdialog and sade for all architectures
#				# without making clean
# sh tools/tinder.sh gnu/lib/libdialog usr.sbin/sade TARGETS="amd64 i386"
#				# build libdialog and sade only for amd64 and i386
#

if [ $# -eq 0 ]; then
	echo 1>&2 "Usage: `basename $0` [MAKEVAR=value...] path..."
	exit 1
fi

# MAKE_ARGS is intentionally not reset to allow caller to specify additional MAKE_ARGS
SUBDIR=
for i in "$@"; do
	case "$i" in
	*=*)
		MAKE_ARGS="$MAKE_ARGS $i"
		;;
	*)
		SUBDIR="$SUBDIR $i"
		;;
	esac
done
make tinderbox UNIVERSE_TARGET="_cleanobj _obj _depend everything" $MAKE_ARGS SUBDIR_OVERRIDE="$SUBDIR"