
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
# The data below is taken from the windiss.dld linker script that comes with
# the Diab linker.TEXT_START_ADDR=0x100000
DATA_START_SYMBOLS='__DATA_ROM = .; __DATA_RAM = .;'
SDATA_START_SYMBOLS='_SDA_BASE_ = .; _gp = . + 0x7ff0;'
SDATA2_START_SYMBOLS='_SDA2_BASE_ = .;'
EXECUTABLE_SYMBOLS='__HEAP_START = .; __SP_INIT = 0x800000; __SP_END = __SP_INIT - 0x20000; __HEAP_END = __SP_END; __DATA_END = _edata; __BSS_START = __bss_start; __BSS_END = _end; __HEAP_START = _end;'

# The Diab tools use a different init/fini convention.  Initialization code
# is place in sections named ".init$NN".  These sections are then concatenated
# into the .init section.  It is important that .init$00 be first and .init$99
# be last. The other sections should be sorted, but the current linker script
# parse does not seem to allow that with the SORT keyword in this context.
INIT_START='*(.init$00); *(.init$0[1-9]); *(.init$[1-8][0-9]); *(.init$9[0-8])'
INIT_END='*(.init$99)'
FINI_START='*(.fini$00); *(.fini$0[1-9]); *(.fini$[1-8][0-9]); *(.fini$9[0-8])'
FINI_END='*(.fini$99)'