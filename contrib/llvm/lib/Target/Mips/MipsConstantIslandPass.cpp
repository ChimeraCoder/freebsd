
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

#define DEBUG_TYPE "mips-constant-islands"

#include "Mips.h"
#include "MCTargetDesc/MipsBaseInfo.h"
#include "MipsTargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegisterInfo.h"

using namespace llvm;

namespace {
  typedef MachineBasicBlock::iterator Iter;
  typedef MachineBasicBlock::reverse_iterator ReverseIter;

  class MipsConstantIslands : public MachineFunctionPass {

  public:
    static char ID;
    MipsConstantIslands(TargetMachine &tm)
      : MachineFunctionPass(ID), TM(tm),
        TII(static_cast<const MipsInstrInfo*>(tm.getInstrInfo())),
        IsPIC(TM.getRelocationModel() == Reloc::PIC_),
        ABI(TM.getSubtarget<MipsSubtarget>().getTargetABI()) {}

    virtual const char *getPassName() const {
      return "Mips Constant Islands";
    }

    bool runOnMachineFunction(MachineFunction &F);

  private:


    const TargetMachine &TM;
    const MipsInstrInfo *TII;
    bool IsPIC;
    unsigned ABI;

  };

  char MipsConstantIslands::ID = 0;
} // end of anonymous namespace

/// createMipsLongBranchPass - Returns a pass that converts branches to long
/// branches.
FunctionPass *llvm::createMipsConstantIslandPass(MipsTargetMachine &tm) {
  return new MipsConstantIslands(tm);
}

bool MipsConstantIslands::runOnMachineFunction(MachineFunction &F) {
  return true;
}