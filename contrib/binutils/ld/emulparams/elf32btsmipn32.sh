
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
# elf32ltsmipn32.sh
. ${srcdir}/emulparams/elf32bmipn32-defs.sh
OUTPUT_FORMAT="elf32-ntradbigmips"
BIG_OUTPUT_FORMAT="elf32-ntradbigmips"
LITTLE_OUTPUT_FORMAT="elf32-ntradlittlemips"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"

# Magic sections.
OTHER_TEXT_SECTIONS='*(.mips16.fn.*) *(.mips16.call.*)'
OTHER_SECTIONS='
  .gptab.sdata : { *(.gptab.data) *(.gptab.sdata) }
  .gptab.sbss : { *(.gptab.bss) *(.gptab.sbss) }
'