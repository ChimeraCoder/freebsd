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
# This file is in the public domain

set -e

T=/tmp/_$$

cd /usr/src
make showconfig __MAKE_CONF=/dev/null SRCCONF=/dev/null |
	sort |
	sed '
		s/^MK_//
		s/=//
	' | awk '
	$2 == "yes"	{ printf "WITHOUT_%s\n", $1 }
	$2 == "no"	{ printf "WITH_%s\n", $1 }
	'