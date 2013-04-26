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

if smbios_mem=$(kenv -q smbios.memory.enabled); then
	smbios_mem=$(expr $smbios_mem / 1024)
else
	smbios_mem=0
fi
realmem=$(expr $(sysctl -n hw.realmem) / 1048576)

if [ $smbios_mem -gt $realmem ]; then
	echo $smbios_mem
else
	echo $realmem
fi