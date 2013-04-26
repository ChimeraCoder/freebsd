
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
# $FreeBSD$. ${srcdir}/emulparams/elf64_ia64.sh
TEXT_START_ADDR="0x0000000100000000"
unset DATA_ADDR
unset SMALL_DATA_CTOR
unset SMALL_DATA_DTOR
. ${srcdir}/emulparams/elf_fbsd.sh
OUTPUT_FORMAT="elf64-ia64-freebsd"