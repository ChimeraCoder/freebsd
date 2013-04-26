
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
# elf32_sparc_vxworks.sh
SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-sparc"
TEXT_START_ADDR=0x10000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
NONPAGED_TEXT_START_ADDR=0x10000
ALIGNMENT=8
ARCH=sparc
MACHINE=
TEMPLATE_NAME=elf32
DATA_PLT=
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
NO_SMALL_DATA=yes