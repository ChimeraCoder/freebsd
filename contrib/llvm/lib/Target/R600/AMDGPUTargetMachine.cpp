
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

#include "AMDGPUTargetMachine.h"
#include "AMDGPU.h"
#include "R600ISelLowering.h"
#include "R600InstrInfo.h"
#include "R600MachineScheduler.h"
#include "SIISelLowering.h"
#include "SIInstrInfo.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include <llvm/CodeGen/Passes.h>

using namespace llvm;

extern "C" void LLVMInitializeR600Target() {
  // Register the target
  RegisterTargetMachine<AMDGPUTargetMachine> X(TheAMDGPUTarget);
}

static ScheduleDAGInstrs *createR600MachineScheduler(MachineSchedContext *C) {
  return new ScheduleDAGMI(C, new R600SchedStrategy());
}

static MachineSchedRegistry
SchedCustomRegistry("r600", "Run R600's custom scheduler",
                    createR600MachineScheduler);

AMDGPUTargetMachine::AMDGPUTargetMachine(const Target &T, StringRef TT,
    StringRef CPU, StringRef FS,
  TargetOptions Options,
  Reloc::Model RM, CodeModel::Model CM,
  CodeGenOpt::Level OptLevel
)
:
  LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OptLevel),
  Subtarget(TT, CPU, FS),
  Layout(Subtarget.getDataLayout()),
  FrameLowering(TargetFrameLowering::StackGrowsUp,
      Subtarget.device()->getStackAlignment(), 0),
  IntrinsicInfo(this),
  InstrItins(&Subtarget.getInstrItineraryData()) {
  // TLInfo uses InstrInfo so it must be initialized after.
  if (Subtarget.device()->getGeneration() <= AMDGPUDeviceInfo::HD6XXX) {
    InstrInfo = new R600InstrInfo(*this);
    TLInfo = new R600TargetLowering(*this);
  } else {
    InstrInfo = new SIInstrInfo(*this);
    TLInfo = new SITargetLowering(*this);
  }
}

AMDGPUTargetMachine::~AMDGPUTargetMachine() {
}

namespace {
class AMDGPUPassConfig : public TargetPassConfig {
public:
  AMDGPUPassConfig(AMDGPUTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {
    const AMDGPUSubtarget &ST = TM->getSubtarget<AMDGPUSubtarget>();
    if (ST.device()->getGeneration() <= AMDGPUDeviceInfo::HD6XXX) {
      enablePass(&MachineSchedulerID);
      MachineSchedRegistry::setDefault(createR600MachineScheduler);
    }
  }

  AMDGPUTargetMachine &getAMDGPUTargetMachine() const {
    return getTM<AMDGPUTargetMachine>();
  }

  virtual bool addPreISel();
  virtual bool addInstSelector();
  virtual bool addPreRegAlloc();
  virtual bool addPostRegAlloc();
  virtual bool addPreSched2();
  virtual bool addPreEmitPass();
};
} // End of anonymous namespace

TargetPassConfig *AMDGPUTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new AMDGPUPassConfig(this, PM);
}

bool
AMDGPUPassConfig::addPreISel() {
  const AMDGPUSubtarget &ST = TM->getSubtarget<AMDGPUSubtarget>();
  if (ST.device()->getGeneration() > AMDGPUDeviceInfo::HD6XXX) {
    addPass(createAMDGPUStructurizeCFGPass());
    addPass(createSIAnnotateControlFlowPass());
  }
  return false;
}

bool AMDGPUPassConfig::addInstSelector() {
  addPass(createAMDGPUPeepholeOpt(*TM));
  addPass(createAMDGPUISelDag(getAMDGPUTargetMachine()));

  const AMDGPUSubtarget &ST = TM->getSubtarget<AMDGPUSubtarget>();
  if (ST.device()->getGeneration() <= AMDGPUDeviceInfo::HD6XXX) {
    // This callbacks this pass uses are not implemented yet on SI.
    addPass(createAMDGPUIndirectAddressingPass(*TM));
  }
  return false;
}

bool AMDGPUPassConfig::addPreRegAlloc() {
  addPass(createAMDGPUConvertToISAPass(*TM));
  return false;
}

bool AMDGPUPassConfig::addPostRegAlloc() {
  const AMDGPUSubtarget &ST = TM->getSubtarget<AMDGPUSubtarget>();

  if (ST.device()->getGeneration() > AMDGPUDeviceInfo::HD6XXX) {
    addPass(createSIInsertWaits(*TM));
  }
  return false;
}

bool AMDGPUPassConfig::addPreSched2() {

  addPass(&IfConverterID);
  return false;
}

bool AMDGPUPassConfig::addPreEmitPass() {
  const AMDGPUSubtarget &ST = TM->getSubtarget<AMDGPUSubtarget>();
  if (ST.device()->getGeneration() <= AMDGPUDeviceInfo::HD6XXX) {
    addPass(createAMDGPUCFGPreparationPass(*TM));
    addPass(createAMDGPUCFGStructurizerPass(*TM));
    addPass(createR600EmitClauseMarkers(*TM));
    addPass(createR600ExpandSpecialInstrsPass(*TM));
    addPass(createR600ControlFlowFinalizer(*TM));
    addPass(&FinalizeMachineBundlesID);
  } else {
    addPass(createSILowerControlFlowPass(*TM));
  }

  return false;
}