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
D1="/usr/obj/nanobsd.rescue_i386"
D2="/usr/obj/nanobsd.rescue_amd64"

MD=`mdconfig -a -t vnode -f ${D1}/_.disk.full`

dd if=${D2}/_.disk.image of=/dev/${MD}s2 bs=128k
tunefs -L rescues2a /dev/${MD}s2a
mount /dev/${MD}s2a ${D1}/_.mnt

sed -i "" -e 's/rescues1/rescues2/' ${D1}/_.mnt/conf/base/etc/fstab
sed -i "" -e 's/rescues1/rescues2/' ${D1}/_.mnt/etc/fstab

umount ${D1}/_.mnt

mdconfig -d -u ${MD}