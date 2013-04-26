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
# This is a simple wrapper for running the MegaCLI tool for Linux.  It assumes
# that the MegaCLI binary has been installed in /compat/linux/usr/sbin/MegaCli.
# The binary must also have been branded appropriately, and the COMPAT_LINUX,
# LINPROCFS, and LINSYSFS options must be enabled.  It is best to enable the
# COMPAT_LINUX option and compile the mfi driver into the kernel, instead of
# loading it as a module, to ensure that all of the required driver bits are
# enabled.
#

megacli=/usr/sbin/MegaCli
linuxdir=/compat/linux
osrelease=2.6.12

devfsmount=`mount |grep $linuxdir/dev | awk '{print $3}'`
if [ "X$devfsmount" = "X" ]; then
	mount -t devfs devfs $linuxdir/dev
fi

procfsmount=`mount |grep $linuxdir/proc | awk '{print $3}'`
if [ "X$procfsmount" = "X" ]; then
	mount -t linprocfs linprocfs $linuxdir/proc
fi

sysfsmount=`mount |grep $linuxdir/sys | awk '{print $3}'`
if [ "X$sysfsmount" = "X" ]; then
	mount -t linsysfs linsysfs $linuxdir/sys
fi

linuxver=`sysctl -n compat.linux.osrelease | cut -d . -f 1,2`
if [ "X$linuxver" != "X2.6" ]; then
	sysctl compat.linux.osrelease=$osrelease
fi

chroot $linuxdir $megacli $@