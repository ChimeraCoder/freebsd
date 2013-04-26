
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PPCMCAsmInfo.h"
using namespace llvm;

void PPCMCAsmInfoDarwin::anchor() { }

PPCMCAsmInfoDarwin::PPCMCAsmInfoDarwin(bool is64Bit) {
  if (is64Bit) {
    PointerSize = CalleeSaveStackSlotSize = 8;
  }
  IsLittleEndian = false;

  PCSymbol = ".";
  CommentString = ";";
  ExceptionsType = ExceptionHandling::DwarfCFI;

  if (!is64Bit)
    Data64bitsDirective = 0;      // We can't emit a 64-bit unit in PPC32 mode.

  AssemblerDialect = 1;           // New-Style mnemonics.
  SupportsDebugInformation= true; // Debug information.
}

void PPCLinuxMCAsmInfo::anchor() { }

PPCLinuxMCAsmInfo::PPCLinuxMCAsmInfo(bool is64Bit) {
  if (is64Bit) {
    PointerSize = CalleeSaveStackSlotSize = 8;
  }
  IsLittleEndian = false;

  // ".comm align is in bytes but .align is pow-2."
  AlignmentIsInBytes = false;

  CommentString = "#";
  GlobalPrefix = "";
  PrivateGlobalPrefix = ".L";
  WeakRefDirective = "\t.weak\t";
  
  // Uses '.section' before '.bss' directive
  UsesELFSectionDirectiveForBSS = true;  

  // Debug Information
  SupportsDebugInformation = true;

  PCSymbol = ".";

  // Set up DWARF directives
  HasLEB128 = true;  // Target asm supports leb128 directives (little-endian)

  // Exceptions handling
  ExceptionsType = ExceptionHandling::DwarfCFI;
    
  ZeroDirective = "\t.space\t";
  Data64bitsDirective = is64Bit ? "\t.quad\t" : 0;
  AssemblerDialect = 0;           // Old-Style mnemonics.
}