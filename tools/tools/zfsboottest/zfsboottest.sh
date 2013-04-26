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

if [ $# -ne 1 ]; then
	echo "usage: zfsboottest.sh <pool>" >&2
	exit 1
fi

which -s zfsboottest
if [ $? -eq 0 ]; then
	zfsboottest="zfsboottest"
else
	if [ ! -x "/usr/src/tools/tools/zfsboottest/zfsboottest" ]; then
		echo "Unable to find \"zfsboottest\" utility." >&2
		exit 1
	fi
	zfsboottest="/usr/src/tools/tools/zfsboottest/zfsboottest"
fi

startdir="/boot"

pool="${1}"
zpool list "${pool}" >/dev/null 2>&1
if [ $? -ne 0 ]; then
	echo "No such pool \"${pool}\"." >&2
	exit 1
fi
bootfs=`zpool get bootfs "${pool}" | tail -1 | awk '{print $3}'`
if [ "${bootfs}" = "-" ]; then
	bootfs="${pool}"
fi
# Dataset's mountpoint property should be set to 'legacy'.
if [ "`zfs get -H -o value mountpoint ${bootfs}`" != "legacy" ]; then
	echo "The \"mountpoint\" property of dataset \"${bootfs}\" should be set to \"legacy\"." >&2
	exit 1
fi
mountpoint=`df -t zfs "${bootfs}" 2>/dev/null | tail -1 | awk '{print $6}'`
if [ -z "${mountpoint}" ]; then
	echo "The \"${bootfs}\" dataset is not mounted." >&2
	exit 1
fi
if [ ! -d "${mountpoint}${startdir}" ]; then
	echo "The \"${mountpoint}${startdir}\" directory doesn't exist." >&2
	exit 1
fi
# To be able to mount root ZFS file system we need either /etc/fstab entry
# or vfs.root.mountfrom variable set in /boot/loader.conf.
egrep -q '^'"${bootfs}"'[[:space:]]+/[[:space:]]+zfs[[:space:]]+' "${mountpoint}/etc/fstab" 2>/dev/null
if [ $? -ne 0 ]; then
	egrep -q 'vfs.root.mountfrom="?'"zfs:${bootfs}"'"?[[:space:]]*$' "${mountpoint}/boot/loader.conf" 2>/dev/null
	if [ $? -ne 0 ]; then
		echo "To be able to boot from \"${bootfs}\", you need to declare" >&2
		echo "\"${bootfs}\" as being root file system in ${mountpoint}/etc/fstab" >&2
		echo "or add \"vfs.root.mountfrom\" variable set to \"zfs:${bootfs}\" to" >&2
		echo "${mountpoint}/boot/loader.conf." >&2
		exit 1
	fi
fi
vdevs=""
for vdev in `zpool status "${pool}" | grep ONLINE | awk '{print $1}'`; do
	vdev="/dev/${vdev#/dev/}"
	if [ -c "${vdev}" ]; then
		if [ -z "${vdevs}" ]; then
			vdevs="${vdev}"
		else
			vdevs="${vdevs} ${vdev}"
		fi
	fi
done

list0=`mktemp /tmp/zfsboottest.XXXXXXXXXX`
if [ $? -ne 0 ]; then
	echo "Unable to create temporary file." >&2
	exit 1
fi
list1=`mktemp /tmp/zfsboottest.XXXXXXXXXX`
if [ $? -ne 0 ]; then
	echo "Unable to create temporary file." >&2
	rm -f "${list0}"
	exit 1
fi

echo "zfsboottest.sh is reading all the files in ${mountpoint}${startdir} using"
echo "boot code and using file system code."
echo "It calculates MD5 checksums for all the files and will compare them."
echo "If all files can be properly read using boot code, it is very likely you"
echo "will be able to boot from \"${pool}\" pool>:> Good luck!"
echo

"${zfsboottest}" ${vdevs} - `find "${mountpoint}${startdir}" -type f | sed "s@^${mountpoint}@@"` | egrep '^[0-9a-z]{32} /' | sort -k 2 >"${list0}"
find "${mountpoint}${startdir}" -type f | xargs md5 -r | sed "s@ ${mountpoint}@ @" | egrep '^[0-9a-z]{32} /' | sort -k 2 >"${list1}"

diff -u "${list0}" "${list1}"
ec=$?

rm -f "${list0}" "${list1}"

if [ $? -ne 0 ]; then
	echo >&2
	echo "You may not be able to boot." >&2
	exit 1
fi

echo "OK"