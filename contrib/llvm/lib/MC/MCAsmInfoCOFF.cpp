
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

#include "llvm/MC/MCAsmInfoCOFF.h"
using namespace llvm;

void MCAsmInfoCOFF::anchor() { }

MCAsmInfoCOFF::MCAsmInfoCOFF() {
  GlobalPrefix = "_";
  // MingW 4.5 and later support .comm with log2 alignment, but .lcomm uses byte
  // alignment.
  COMMDirectiveAlignmentIsInBytes = false;
  LCOMMDirectiveAlignmentType = LCOMM::ByteAlignment;
  HasDotTypeDotSizeDirective = false;
  HasSingleParameterDotFile = false;
  PrivateGlobalPrefix = "L";  // Prefix for private global symbols
  WeakRefDirective = "\t.weak\t";
  LinkOnceDirective = "\t.linkonce discard\n";

  // Doesn't support visibility:
  HiddenVisibilityAttr = HiddenDeclarationVisibilityAttr = MCSA_Invalid;
  ProtectedVisibilityAttr = MCSA_Invalid;

  // Set up DWARF directives
  HasLEB128 = true;  // Target asm supports leb128 directives (little-endian)
  SupportsDebugInformation = true;
  DwarfSectionOffsetDirective = "\t.secrel32\t";
  HasMicrosoftFastStdCallMangling = true;
}

void MCAsmInfoMicrosoft::anchor() { }

MCAsmInfoMicrosoft::MCAsmInfoMicrosoft() {
  AllowQuotesInName = true;
}

void MCAsmInfoGNUCOFF::anchor() { }

MCAsmInfoGNUCOFF::MCAsmInfoGNUCOFF() {

}