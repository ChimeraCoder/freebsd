
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

#include "MipsMCAsmInfo.h"
#include "llvm/ADT/Triple.h"

using namespace llvm;

void MipsMCAsmInfo::anchor() { }

MipsMCAsmInfo::MipsMCAsmInfo(const Target &T, StringRef TT) {
  Triple TheTriple(TT);
  if ((TheTriple.getArch() == Triple::mips) ||
      (TheTriple.getArch() == Triple::mips64))
    IsLittleEndian = false;

  if ((TheTriple.getArch() == Triple::mips64el) ||
      (TheTriple.getArch() == Triple::mips64)) {
    PointerSize = CalleeSaveStackSlotSize = 8;
  }

  AlignmentIsInBytes          = false;
  Data16bitsDirective         = "\t.2byte\t";
  Data32bitsDirective         = "\t.4byte\t";
  Data64bitsDirective         = "\t.8byte\t";
  PrivateGlobalPrefix         = "$";
  CommentString               = "#";
  ZeroDirective               = "\t.space\t";
  GPRel32Directive            = "\t.gpword\t";
  GPRel64Directive            = "\t.gpdword\t";
  WeakRefDirective            = "\t.weak\t";
  DebugLabelSuffix            = "=.";
  SupportsDebugInformation = true;
  ExceptionsType = ExceptionHandling::DwarfCFI;
  HasLEB128 = true;
  DwarfRegNumForCFI = true;
}