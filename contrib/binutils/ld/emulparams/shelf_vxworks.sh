
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
# If you change this file, please also look at files which source this one:
# shlelf_vxworks.sh
SCRIPT_NAME=elf
BIG_OUTPUT_FORMAT="elf32-sh-vxworks"
LITTLE_OUTPUT_FORMAT="elf32-shl-vxworks"
OUTPUT_FORMAT="$BIG_OUTPUT_FORMAT"
TEXT_START_ADDR=0x1000
MAXPAGESIZE='CONSTANT (MAXPAGESIZE)'
ARCH=sh
MACHINE=
TEMPLATE_NAME=elf32
GENERATE_SHLIB_SCRIPT=yes
ENTRY=__start
SYMPREFIX=_
GOT=".got          ${RELOCATING-0} : {
  PROVIDE(__GLOBAL_OFFSET_TABLE_ = .);
  *(.got.plt) *(.got) }"
. ${srcdir}/emulparams/vxworks.sh