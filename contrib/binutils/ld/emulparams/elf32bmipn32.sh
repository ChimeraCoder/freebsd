
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
# IRIX6 defines these symbols.  0x34 is the size of the ELF header.EXECUTABLE_SYMBOLS="
  __dso_displacement = 0;
  __elf_header = ${TEXT_START_ADDR};
  __program_header_table = ${TEXT_START_ADDR} + 0x34;
"

# There are often dynamic relocations against the .rodata section.
# Setting DT_TEXTREL in the .dynamic section does not convince the
# IRIX6 linker to permit relocations against the text segment.
# Following the IRIX linker, we simply put .rodata in the data
# segment.
WRITABLE_RODATA=

EXTRA_EM_FILE=irix