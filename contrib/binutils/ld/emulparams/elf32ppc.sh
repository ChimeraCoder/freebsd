
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
# elf32lppcnto.sh elf32lppc.sh elf32ppclinux.sh elf32ppcnto.sh 
# elf32ppcsim.sh
. ${srcdir}/emulparams/elf32ppccommon.sh
# Yes, we want duplicate .got and .plt sections.  The linker chooses the
# appropriate one magically in ppc_after_open
DATA_GOT=
SDATA_GOT=
SEPARATE_GOTPLT=0
BSS_PLT=
GOT=".got          ${RELOCATING-0} : SPECIAL { *(.got) }"
PLT=".plt          ${RELOCATING-0} : SPECIAL { *(.plt) }"
GOTPLT="${PLT}"
OTHER_TEXT_SECTIONS="*(.glink)"
EXTRA_EM_FILE=ppc32elf