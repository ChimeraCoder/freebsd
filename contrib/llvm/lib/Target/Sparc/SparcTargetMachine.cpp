
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

#include "SparcTargetMachine.h"
#include "Sparc.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeSparcTarget() {
  // Register the target.
  RegisterTargetMachine<SparcV8TargetMachine> X(TheSparcTarget);
  RegisterTargetMachine<SparcV9TargetMachine> Y(TheSparcV9Target);
}

/// SparcTargetMachine ctor - Create an ILP32 architecture model
///
SparcTargetMachine::SparcTargetMachine(const Target &T, StringRef TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Reloc::Model RM, CodeModel::Model CM,
                                       CodeGenOpt::Level OL,
                                       bool is64bit)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS, is64bit),
    DL(Subtarget.getDataLayout()),
    InstrInfo(Subtarget),
    TLInfo(*this), TSInfo(*this),
    FrameLowering(Subtarget) {
}

namespace {
/// Sparc Code Generator Pass Configuration Options.
class SparcPassConfig : public TargetPassConfig {
public:
  SparcPassConfig(SparcTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  SparcTargetMachine &getSparcTargetMachine() const {
    return getTM<SparcTargetMachine>();
  }

  virtual bool addInstSelector();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *SparcTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SparcPassConfig(this, PM);
}

bool SparcPassConfig::addInstSelector() {
  addPass(createSparcISelDag(getSparcTargetMachine()));
  return false;
}

/// addPreEmitPass - This pass may be implemented by targets that want to run
/// passes immediately before machine code is emitted.  This should return
/// true if -print-machineinstrs should print out the code after the passes.
bool SparcPassConfig::addPreEmitPass(){
  addPass(createSparcFPMoverPass(getSparcTargetMachine()));
  addPass(createSparcDelaySlotFillerPass(getSparcTargetMachine()));
  return true;
}

void SparcV8TargetMachine::anchor() { }

SparcV8TargetMachine::SparcV8TargetMachine(const Target &T,
                                           StringRef TT, StringRef CPU,
                                           StringRef FS,
                                           const TargetOptions &Options,
                                           Reloc::Model RM,
                                           CodeModel::Model CM,
                                           CodeGenOpt::Level OL)
  : SparcTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, false) {
}

void SparcV9TargetMachine::anchor() { }

SparcV9TargetMachine::SparcV9TargetMachine(const Target &T,
                                           StringRef TT,  StringRef CPU,
                                           StringRef FS,
                                           const TargetOptions &Options,
                                           Reloc::Model RM,
                                           CodeModel::Model CM,
                                           CodeGenOpt::Level OL)
  : SparcTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, true) {
}