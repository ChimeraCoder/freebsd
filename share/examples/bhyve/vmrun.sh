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

LOADER=/usr/sbin/bhyveload
BHYVECTL=/usr/sbin/bhyvectl
FBSDRUN=/usr/sbin/bhyve

DEFAULT_MEMSIZE=512
DEFAULT_CPUS=2
DEFAULT_TAPDEV=tap0

DEFAULT_VIRTIO_DISK="./diskdev"
DEFAULT_ISOFILE="./release.iso"

usage() {
	echo "Usage: vmrun.sh [-hai][-g <gdbport>][-m <memsize>][-d <disk file>][-I <location of installation iso>][-t <tapdev>] <vmname>"
	echo "       -h: display this help message"
	echo "       -a: force memory mapped local apic access"
	echo "       -c: number of virtual cpus (default is ${DEFAULT_CPUS})"
	echo "       -d: virtio diskdev file (default is ${DEFAULT_VIRTIO_DISK})"
	echo "       -g: listen for connection from kgdb at <gdbport>"
	echo "       -i: force boot of the Installation CDROM image"
	echo "       -I: Installation CDROM image location (default is ${DEFAULT_ISOFILE})"
	echo "       -m: memory size in MB (default is ${DEFAULT_MEMSIZE}MB)"
	echo "       -t: tap device for virtio-net (default is $DEFAULT_TAPDEV)"
	echo ""
	echo "       This script needs to be executed with superuser privileges"
	echo ""
	exit 1
}

if [ `id -u` -ne 0 ]; then
	usage
fi

kldstat -n vmm > /dev/null 2>&1 
if [ $? -ne 0 ]; then
	echo "vmm.ko is not loaded!"
	exit 1
fi

force_install=0
isofile=${DEFAULT_ISOFILE}
memsize=${DEFAULT_MEMSIZE}
cpus=${DEFAULT_CPUS}
virtio_diskdev=${DEFAULT_VIRTIO_DISK}
tapdev=${DEFAULT_TAPDEV}
apic_opt=""
gdbport=0

while getopts haic:g:I:m:d:t: c ; do
	case $c in
	h)
		usage
		;;
	a)
		apic_opt="-a"
		;;
	d)
		virtio_diskdev=${OPTARG}
		;;
	g)	gdbport=${OPTARG}
		;;
	i)
		force_install=1
		;;
	I)
		isofile=${OPTARG}
		;;
	c)
		cpus=${OPTARG}
		;;
	m)
		memsize=${OPTARG}
		;;
	t)
		tapdev=${OPTARG}
		;;
	\?)
		usage
		;;
	esac
done

shift $((${OPTIND} - 1))

if [ $# -ne 1 ]; then
	usage
fi

vmname="$1"

# Create the virtio diskdev file if needed
if [ ! -f ${virtio_diskdev} ]; then
	echo "virtio disk device file \"${virtio_diskdev}\" does not exist."
	echo "Creating it ..."
	truncate -s 8G ${virtio_diskdev} > /dev/null
fi

if [ ! -r ${virtio_diskdev} ]; then
	echo "virtio disk device file \"${virtio_diskdev}\" is not readable"
	exit 1
fi

if [ ! -w ${virtio_diskdev} ]; then
	echo "virtio disk device file \"${virtio_diskdev}\" is not writable"
	exit 1
fi

echo "Launching virtual machine \"$vmname\" ..."

while [ 1 ]; do
	${BHYVECTL} --vm=${vmname} --destroy > /dev/null 2>&1

	file ${virtio_diskdev} | grep ": x86 boot sector" > /dev/null
	rc=$?
	if [ $rc -ne 0 ]; then
		file ${virtio_diskdev} | grep ": Unix Fast File sys" > /dev/null
		rc=$?
	fi
	if [ $rc -ne 0 ]; then
		need_install=1
	else
		need_install=0
	fi

	if [ $force_install -eq 1 -o $need_install -eq 1 ]; then
		if [ ! -r ${isofile} ]; then
			echo -n "Installation CDROM image \"${isofile}\" "
			echo    "is not readable"
			exit 1
		fi
		BOOTDISK=${isofile}
		installer_opt="-s 3:0,virtio-blk,${BOOTDISK}"
	else
		BOOTDISK=${virtio_diskdev}
		installer_opt=""
	fi

	${LOADER} -m ${memsize} -d ${BOOTDISK} ${vmname}
	if [ $? -ne 0 ]; then
		break
	fi

	${FBSDRUN} -c ${cpus} -m ${memsize} ${apic_opt} -AI -H -P	\
		-g ${gdbport}						\
		-s 0:0,hostbridge					\
		-s 1:0,virtio-net,${tapdev}				\
		-s 2:0,virtio-blk,${virtio_diskdev}			\
		${installer_opt}					\
		-S 31,uart,stdio					\
		${vmname}
	if [ $? -ne 0 ]; then
		break
	fi
done

exit 99