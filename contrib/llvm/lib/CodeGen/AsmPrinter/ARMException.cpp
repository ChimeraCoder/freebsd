
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

#include "DwarfException.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetRegisterInfo.h"
using namespace llvm;

static cl::opt<bool>
EnableARMEHABIDescriptors("arm-enable-ehabi-descriptors", cl::Hidden,
  cl::desc("Generate ARM EHABI tables with unwinding descriptors"),
  cl::init(false));


ARMException::ARMException(AsmPrinter *A)
  : DwarfException(A) {}

ARMException::~ARMException() {}

void ARMException::EndModule() {
}

/// BeginFunction - Gather pre-function exception information. Assumes it's
/// being emitted immediately after the function entry point.
void ARMException::BeginFunction(const MachineFunction *MF) {
  Asm->OutStreamer.EmitFnStart();
  if (Asm->MF->getFunction()->needsUnwindTableEntry())
    Asm->OutStreamer.EmitLabel(Asm->GetTempSymbol("eh_func_begin",
                                                  Asm->getFunctionNumber()));
}

/// EndFunction - Gather and emit post-function exception information.
///
void ARMException::EndFunction() {
  if (!Asm->MF->getFunction()->needsUnwindTableEntry())
    Asm->OutStreamer.EmitCantUnwind();
  else {
    Asm->OutStreamer.EmitLabel(Asm->GetTempSymbol("eh_func_end",
                                                  Asm->getFunctionNumber()));

    if (EnableARMEHABIDescriptors) {
      // Map all labels and get rid of any dead landing pads.
      MMI->TidyLandingPads();

      if (!MMI->getLandingPads().empty()) {
        // Emit references to personality.
        if (const Function * Personality =
            MMI->getPersonalities()[MMI->getPersonalityIndex()]) {
          MCSymbol *PerSym = Asm->Mang->getSymbol(Personality);
          Asm->OutStreamer.EmitSymbolAttribute(PerSym, MCSA_Global);
          Asm->OutStreamer.EmitPersonality(PerSym);
        }

        // Emit .handlerdata directive.
        Asm->OutStreamer.EmitHandlerData();

        // Emit actual exception table
        EmitExceptionTable();
      }
    }
  }

  Asm->OutStreamer.EmitFnEnd();
}

void ARMException::EmitTypeInfos(unsigned TTypeEncoding) {
  const std::vector<const GlobalVariable *> &TypeInfos = MMI->getTypeInfos();
  const std::vector<unsigned> &FilterIds = MMI->getFilterIds();

  bool VerboseAsm = Asm->OutStreamer.isVerboseAsm();

  int Entry = 0;
  // Emit the Catch TypeInfos.
  if (VerboseAsm && !TypeInfos.empty()) {
    Asm->OutStreamer.AddComment(">> Catch TypeInfos <<");
    Asm->OutStreamer.AddBlankLine();
    Entry = TypeInfos.size();
  }

  for (std::vector<const GlobalVariable *>::const_reverse_iterator
         I = TypeInfos.rbegin(), E = TypeInfos.rend(); I != E; ++I) {
    const GlobalVariable *GV = *I;
    if (VerboseAsm)
      Asm->OutStreamer.AddComment("TypeInfo " + Twine(Entry--));
    Asm->EmitTTypeReference(GV, TTypeEncoding);
  }

  // Emit the Exception Specifications.
  if (VerboseAsm && !FilterIds.empty()) {
    Asm->OutStreamer.AddComment(">> Filter TypeInfos <<");
    Asm->OutStreamer.AddBlankLine();
    Entry = 0;
  }
  for (std::vector<unsigned>::const_iterator
         I = FilterIds.begin(), E = FilterIds.end(); I < E; ++I) {
    unsigned TypeID = *I;
    if (VerboseAsm) {
      --Entry;
      if (TypeID != 0)
        Asm->OutStreamer.AddComment("FilterInfo " + Twine(Entry));
    }

    Asm->EmitTTypeReference((TypeID == 0 ? 0 : TypeInfos[TypeID - 1]),
                            TTypeEncoding);
  }
}