
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

#include "AArch64.h"
#include "AArch64TargetMachine.h"
#include "MCTargetDesc/AArch64MCTargetDesc.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

extern "C" void LLVMInitializeAArch64Target() {
  RegisterTargetMachine<AArch64TargetMachine> X(TheAArch64Target);
}

AArch64TargetMachine::AArch64TargetMachine(const Target &T, StringRef TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           Reloc::Model RM, CodeModel::Model CM,
                                           CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS),
    InstrInfo(Subtarget),
    DL("e-p:64:64-i64:64:64-i128:128:128-s0:32:32-f128:128:128-n32:64-S128"),
    TLInfo(*this),
    TSInfo(*this),
    FrameLowering(Subtarget) {
}

namespace {
/// AArch64 Code Generator Pass Configuration Options.
class AArch64PassConfig : public TargetPassConfig {
public:
  AArch64PassConfig(AArch64TargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  AArch64TargetMachine &getAArch64TargetMachine() const {
    return getTM<AArch64TargetMachine>();
  }

  const AArch64Subtarget &getAArch64Subtarget() const {
    return *getAArch64TargetMachine().getSubtargetImpl();
  }

  virtual bool addInstSelector();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *AArch64TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new AArch64PassConfig(this, PM);
}

bool AArch64PassConfig::addPreEmitPass() {
  addPass(&UnpackMachineBundlesID);
  addPass(createAArch64BranchFixupPass());
  return true;
}

bool AArch64PassConfig::addInstSelector() {
  addPass(createAArch64ISelDAG(getAArch64TargetMachine(), getOptLevel()));

  // For ELF, cleanup any local-dynamic TLS accesses.
  if (getAArch64Subtarget().isTargetELF() && getOptLevel() != CodeGenOpt::None)
    addPass(createAArch64CleanupLocalDynamicTLSPass());

  return false;
}