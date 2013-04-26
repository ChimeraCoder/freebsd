
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

#include "AMDGPU.h"
#include "AMDGPUInstrInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"

using namespace llvm;

namespace {

class AMDGPUConvertToISAPass : public MachineFunctionPass {

private:
  static char ID;
  TargetMachine &TM;

public:
  AMDGPUConvertToISAPass(TargetMachine &tm) :
    MachineFunctionPass(ID), TM(tm) { }

  virtual bool runOnMachineFunction(MachineFunction &MF);

  virtual const char *getPassName() const {return "AMDGPU Convert to ISA";}

};

} // End anonymous namespace

char AMDGPUConvertToISAPass::ID = 0;

FunctionPass *llvm::createAMDGPUConvertToISAPass(TargetMachine &tm) {
  return new AMDGPUConvertToISAPass(tm);
}

bool AMDGPUConvertToISAPass::runOnMachineFunction(MachineFunction &MF) {
  const AMDGPUInstrInfo * TII =
                      static_cast<const AMDGPUInstrInfo*>(TM.getInstrInfo());

  for (MachineFunction::iterator BB = MF.begin(), BB_E = MF.end();
                                                  BB != BB_E; ++BB) {
    MachineBasicBlock &MBB = *BB;
    for (MachineBasicBlock::iterator I = MBB.begin(), E = MBB.end();
                                                      I != E; ++I) {
      MachineInstr &MI = *I;
      TII->convertToISA(MI, MF, MBB.findDebugLoc(I));
    }
  }
  return false;
}