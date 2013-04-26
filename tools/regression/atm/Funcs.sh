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
# $FreeBSD$
fatal() {
	echo -e "$*" >&2
	exit 1
}

msg() {
	echo -e "$*" >&2
}

usage1() {
	msg "Usage: RunTest.sh [-hq] [-b <localbase>]"
	msg "Options:"
	msg " -h		show this info"
	msg " -b <localbase>	localbase if not /usr/local"
	msg " -q		be quite"
	msg " -u		run user space test, not kernel"
	exit 0
}

parse_options() {
	args=`getopt b:hqu $*`
	if [ $? -ne 0 ] ; then
		fatal "Usage: $0 [-qu] [-b <localbase>]"
	fi

	options=""
	set -- $args
	for i
	do
		case "$i"
		in

		-h)	usage1;;
		-u|-q)	options="$options $i"; shift;;
		-b)	LOCALBASE="$2"; shift; shift;;
		--)	shift; break;;
		esac
	done

	if [ "$LOCALBASE" = "" ] ; then
		LOCALBASE="/usr/local"

		pkg_info -I atmsupport-\* 2>/dev/null >/dev/null
		if [ $? -ne 0 ] ; then
			fatal "Atmsupport package not installed. \
Goto /usr/ports/net/atmsupport,\ntype 'make ; make install ; make clean' \
and re-run this script"
		fi
	fi
}