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

# Resource name as defined in /etc/hast.conf.
resource="test"
# Supported file system types: UFS, ZFS
fstype="UFS"
# ZFS pool name. Required only when fstype == ZFS.
pool="test"
# File system mount point. Required only when fstype == UFS.
mountpoint="/mnt/test"
# Name of HAST provider as defined in /etc/hast.conf.
# Required only when fstype == UFS.
device="/dev/hast/${resource}"

export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin

# KIll UP script if it still runs in the background.
sig="TERM"
for i in `jot 30`; do
	pgid=`pgrep -f ucarp_up.sh | head -1`
	[ -n "${pgid}" ] || break
	kill -${sig} -- -${pgid}
	sig="KILL"
	sleep 1
done
if [ -n "${pgid}" ]; then
	logger -p local0.error -t hast "UCARP UP process for resource ${resource} is still running after 30 seconds."
	exit 1
fi
logger -p local0.debug -t hast "UCARP UP is not running."

case "${fstype}" in
UFS)
	mount | egrep -q "^${device} on "
	if [ $? -eq 0 ]; then
		# Forcibly unmount file system.
		out=`umount -f "${mountpoint}" 2>&1`
		if [ $? -ne 0 ]; then
			logger -p local0.error -t hast "Unable to unmount file system for resource ${resource}: ${out}."
			exit 1
		fi
		logger -p local0.debug -t hast "File system for resource ${resource} unmounted."
	fi
	;;
ZFS)
	zpool list | egrep -q "^${pool} "
	if [ $? -eq 0 ]; then
		# Forcibly export file pool.
		out=`zpool export -f "${pool}" 2>&1`
		if [ $? -ne 0 ]; then
			logger -p local0.error -t hast "Unable to export pool for resource ${resource}: ${out}."
			exit 1
		fi
		logger -p local0.debug -t hast "ZFS pool for resource ${resource} exported."
	fi
	;;
esac

# Change role to secondary for our resource.
out=`hastctl role secondary "${resource}" 2>&1`
if [ $? -ne 0 ]; then
	logger -p local0.error -t hast "Unable to change to role to secondary for resource ${resource}: ${out}."
	exit 1
fi
logger -p local0.debug -t hast "Role for resource ${resource} changed to secondary."

logger -p local0.info -t hast "Successfully switched to secondary for resource ${resource}."

exit 0