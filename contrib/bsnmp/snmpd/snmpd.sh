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
SNMPD=/usr/local/bin/bsnmpd
PID=/var/run/snmpd.pid
CONF=/etc/snmpd.conf

case "$1" in

start)
	if [ -r ${PID} ] ; then
		if kill -0 `cat ${PID}` ; then
			echo "snmpd already running -- pid `cat ${PID}`" >/dev/stderr
			exit 1
		fi
		rm -f ${PID}
	fi
	if ${SNMPD} -c ${CONF} -p ${PID} ; then
		echo "snmpd started"
	fi
	;;

stop)
	if [ -r ${PID} ] ; then
		if kill -0 `cat ${PID}` ; then
			if kill -15 `cat ${PID}` ; then
				echo "snmpd stopped"
				exit 0
			fi
			echo "cannot kill snmpd" >/dev/stderr
			exit 1
		fi
		echo "stale pid file -- removing" >/dev/stderr
		rm -f ${PID}
		exit 1
	fi
	echo "snmpd not running" >/dev/stderr
	;;

status)
	if [ ! -r ${PID} ] ; then
		echo "snmpd not running"
	elif kill -0 `cat ${PID}` ; then
		echo "snmpd pid `cat ${PID}`"
	else
		echo "stale pid file -- pid `cat ${PID}`"
	fi
	;;

*)
	echo "usage: `basename $0` {start|stop|status}"
	exit 1
esac

exit 0