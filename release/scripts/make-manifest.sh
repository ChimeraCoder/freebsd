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

# make-manifest.sh: create checksums and package descriptions for the installer
#
#  Usage: make-manifest.sh foo1.txz foo2.txz ...
#
# The output file looks like this (tab-delimited):
#  foo1.txz SHA256-checksu Number-of-files foo1 Description Install-by-default
#
# $FreeBSD$

desc_base="Base system (MANDATORY)"
desc_kernel="Kernel (MANDATORY)"
desc_doc="Additional documentation"
doc_default=off
desc_games="Games (fortune, etc.)"
desc_lib32="32-bit compatibility libraries"
desc_ports="Ports tree"
desc_src="System source code"
src_default=off

for i in $*; do
	echo "`basename $i`	`sha256 -q $i`	`tar tvf $i | wc -l | tr -d ' '`	`basename $i .txz`	\"`eval echo \\\$desc_$(basename $i .txz)`\"	`eval echo \\\${$(basename $i .txz)_default:-on}`"
done