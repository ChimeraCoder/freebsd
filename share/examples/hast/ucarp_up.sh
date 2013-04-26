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
device="/dev/hast/${resource}"

export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin

# If there is secondary worker process, it means that remote primary process is
# still running. We have to wait for it to terminate.
for i in `jot 30`; do
	pgrep -f "hastd: ${resource} \(secondary\)" >/dev/null 2>&1 || break
	sleep 1
done
if pgrep -f "hastd: ${resource} \(secondary\)" >/dev/null 2>&1; then
	logger -p local0.error -t hast "Secondary process for resource ${resource} is still running after 30 seconds."
	exit 1
fi
logger -p local0.debug -t hast "Secondary process in not running."

# Change role to primary for our resource.
out=`hastctl role primary "${resource}" 2>&1`
if [ $? -ne 0 ]; then
	logger -p local0.error -t hast "Unable to change to role to primary for resource ${resource}: ${out}."
	exit 1
fi
# Wait few seconds for provider to appear.
for i in `jot 50`; do
	[ -c "${device}" ] && break
	sleep 0.1
done
if [ ! -c "${device}" ]; then
	logger -p local0.error -t hast "Device ${device} didn't appear."
	exit 1
fi
logger -p local0.debug -t hast "Role for resource ${resource} changed to primary."

case "${fstype}" in
UFS)
	# Check the file system.
	fsck -y -t ufs "${device}" >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		logger -p local0.error -t hast "File system check for resource ${resource} failed."
		exit 1
	fi
	logger -p local0.debug -t hast "File system check for resource ${resource} finished."
	# Mount the file system.
	out=`mount -t ufs "${device}" "${mountpoint}" 2>&1`
	if [ $? -ne 0 ]; then
		logger -p local0.error -t hast "File system mount for resource ${resource} failed: ${out}."
		exit 1
	fi
	logger -p local0.debug -t hast "File system for resource ${resource} mounted."
	;;
ZFS)
	# Import ZFS pool. Do it forcibly as it remembers hostid of
	# the other cluster node.
	out=`zpool import -f "${pool}" 2>&1`
	if [ $? -ne 0 ]; then
		logger -p local0.error -t hast "ZFS pool import for resource ${resource} failed: ${out}."
		exit 1
	fi
	logger -p local0.debug -t hast "ZFS pool for resource ${resource} imported."
	;;
esac

logger -p local0.info -t hast "Successfully switched to primary for resource ${resource}."

exit 0