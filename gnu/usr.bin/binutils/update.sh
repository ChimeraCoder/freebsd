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
#
# $FreeBSD$
#
# This script installs the main files generated by the binutils
# "configure" scripts and makefiles.  It can be used for upgrading to
# a new version of binutils.
#
# Don't forget to change the VERSION definition in the top level
# "Makefile.inc0".
gnudir=$(pwd)
contribdir="${gnudir}/../../../contrib/binutils"
platform="`uname -m`"

rm -rf build
mkdir build

echo "binutils elf configuration for $platform"

(cd build
    ${contribdir}/configure $platform-unknown-freebsd || exit
    (cd gas
	echo "Updating as"
	make config.h || exit
	cp config.h ${gnudir}/as/$platform/config.h || exit
	)
    (cd ld
	echo "Updating ld"
	make config.h ldemul-list.h || exit
	cp config.h ${gnudir}/ld || exit
	cp ldemul-list.h ${gnudir}/ld/$platform || exit
	)
    (cd bfd
	echo "Updating libbfd"
	make bfd.h config.h || exit
	cp bfd.h ${gnudir}/libbfd/$platform || exit
	cp config.h ${gnudir}/libbfd/$platform || exit
	)
    (cd binutils
	echo "Updating libbinutils"
	make config.h || exit
	cp config.h ${gnudir}/libbinutils/config.h || exit
	)
    )

rm -rf build