
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

#include "MipsTargetMachine.h"
#include "Mips.h"
#include "MipsFrameLowering.h"
#include "MipsInstrInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeMipsTarget() {
  // Register the target.
  RegisterTargetMachine<MipsebTargetMachine> X(TheMipsTarget);
  RegisterTargetMachine<MipselTargetMachine> Y(TheMipselTarget);
  RegisterTargetMachine<MipsebTargetMachine> A(TheMips64Target);
  RegisterTargetMachine<MipselTargetMachine> B(TheMips64elTarget);
}

// DataLayout --> Big-endian, 32-bit pointer/ABI/alignment
// The stack is always 8 byte aligned
// On function prologue, the stack is created by decrementing
// its pointer. Once decremented, all references are done with positive
// offset from the stack/frame pointer, using StackGrowsUp enables
// an easier handling.
// Using CodeModel::Large enables different CALL behavior.
MipsTargetMachine::
MipsTargetMachine(const Target &T, StringRef TT,
                  StringRef CPU, StringRef FS, const TargetOptions &Options,
                  Reloc::Model RM, CodeModel::Model CM,
                  CodeGenOpt::Level OL,
                  bool isLittle)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS, isLittle, RM),
    DL(isLittle ?
               (Subtarget.isABI_N64() ?
                "e-p:64:64:64-i8:8:32-i16:16:32-i64:64:64-f128:128:128-"
                "n32:64-S128" :
                "e-p:32:32:32-i8:8:32-i16:16:32-i64:64:64-n32-S64") :
               (Subtarget.isABI_N64() ?
                "E-p:64:64:64-i8:8:32-i16:16:32-i64:64:64-f128:128:128-"
                "n32:64-S128" :
                "E-p:32:32:32-i8:8:32-i16:16:32-i64:64:64-n32-S64")),
    InstrInfo(MipsInstrInfo::create(*this)),
    FrameLowering(MipsFrameLowering::create(*this, Subtarget)),
    TLInfo(MipsTargetLowering::create(*this)), TSInfo(*this), JITInfo() {
}

void MipsebTargetMachine::anchor() { }

MipsebTargetMachine::
MipsebTargetMachine(const Target &T, StringRef TT,
                    StringRef CPU, StringRef FS, const TargetOptions &Options,
                    Reloc::Model RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL)
  : MipsTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, false) {}

void MipselTargetMachine::anchor() { }

MipselTargetMachine::
MipselTargetMachine(const Target &T, StringRef TT,
                    StringRef CPU, StringRef FS, const TargetOptions &Options,
                    Reloc::Model RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL)
  : MipsTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, true) {}

namespace {
/// Mips Code Generator Pass Configuration Options.
class MipsPassConfig : public TargetPassConfig {
public:
  MipsPassConfig(MipsTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  MipsTargetMachine &getMipsTargetMachine() const {
    return getTM<MipsTargetMachine>();
  }

  const MipsSubtarget &getMipsSubtarget() const {
    return *getMipsTargetMachine().getSubtargetImpl();
  }

  virtual bool addInstSelector();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *MipsTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new MipsPassConfig(this, PM);
}

// Install an instruction selector pass using
// the ISelDag to gen Mips code.
bool MipsPassConfig::addInstSelector() {
  addPass(createMipsISelDag(getMipsTargetMachine()));
  return false;
}

// Implemented by targets that want to run passes immediately before
// machine code is emitted. return true if -print-machineinstrs should
// print out the code after the passes.
bool MipsPassConfig::addPreEmitPass() {
  MipsTargetMachine &TM = getMipsTargetMachine();
  addPass(createMipsDelaySlotFillerPass(TM));

  // NOTE: long branch has not been implemented for mips16.
  if (TM.getSubtarget<MipsSubtarget>().hasStandardEncoding())
    addPass(createMipsLongBranchPass(TM));
  if (TM.getSubtarget<MipsSubtarget>().inMips16Mode())
    addPass(createMipsConstantIslandPass(TM));

  return true;
}

bool MipsTargetMachine::addCodeEmitter(PassManagerBase &PM,
                                       JITCodeEmitter &JCE) {
  // Machine code emitter pass for Mips.
  PM.add(createMipsJITCodeEmitterPass(*this, JCE));
  return false;
}