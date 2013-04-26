
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

#include "AMDGPUMCAsmInfo.h"

using namespace llvm;
AMDGPUMCAsmInfo::AMDGPUMCAsmInfo(const Target &T, StringRef &TT) : MCAsmInfo() {
  HasSingleParameterDotFile = false;
  WeakDefDirective = 0;
  //===------------------------------------------------------------------===//
  HasSubsectionsViaSymbols = true;
  HasMachoZeroFillDirective = false;
  HasMachoTBSSDirective = false;
  HasStaticCtorDtorReferenceInStaticMode = false;
  LinkerRequiresNonEmptyDwarfLines = true;
  MaxInstLength = 16;
  PCSymbol = "$";
  SeparatorString = "\n";
  CommentColumn = 40;
  CommentString = ";";
  LabelSuffix = ":";
  GlobalPrefix = "@";
  PrivateGlobalPrefix = ";.";
  LinkerPrivateGlobalPrefix = "!";
  InlineAsmStart = ";#ASMSTART";
  InlineAsmEnd = ";#ASMEND";
  AssemblerDialect = 0;
  AllowQuotesInName = false;
  AllowNameToStartWithDigit = false;
  AllowPeriodsInName = false;

  //===--- Data Emission Directives -------------------------------------===//
  ZeroDirective = ".zero";
  AsciiDirective = ".ascii\t";
  AscizDirective = ".asciz\t";
  Data8bitsDirective = ".byte\t";
  Data16bitsDirective = ".short\t";
  Data32bitsDirective = ".long\t";
  Data64bitsDirective = ".quad\t";
  GPRel32Directive = 0;
  SunStyleELFSectionSwitchSyntax = true;
  UsesELFSectionDirectiveForBSS = true;
  HasMicrosoftFastStdCallMangling = false;

  //===--- Alignment Information ----------------------------------------===//
  AlignDirective = ".align\t";
  AlignmentIsInBytes = true;
  TextAlignFillValue = 0;

  //===--- Global Variable Emission Directives --------------------------===//
  GlobalDirective = ".global";
  ExternDirective = ".extern";
  HasSetDirective = false;
  HasAggressiveSymbolFolding = true;
  COMMDirectiveAlignmentIsInBytes = false;
  HasDotTypeDotSizeDirective = false;
  HasNoDeadStrip = true;
  HasSymbolResolver = false;
  WeakRefDirective = ".weakref\t";
  LinkOnceDirective = 0;
  //===--- Dwarf Emission Directives -----------------------------------===//
  HasLEB128 = true;
  SupportsDebugInformation = true;
  DwarfSectionOffsetDirective = ".offset";

}

const char*
AMDGPUMCAsmInfo::getDataASDirective(unsigned int Size, unsigned int AS) const {
  return 0;
}

const MCSection*
AMDGPUMCAsmInfo::getNonexecutableStackSection(MCContext &CTX) const {
  return 0;
}