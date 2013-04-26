
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

#define DEBUG_TYPE "expand-isel-pseudos"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

namespace {
  class ExpandISelPseudos : public MachineFunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid
    ExpandISelPseudos() : MachineFunctionPass(ID) {}

  private:
    virtual bool runOnMachineFunction(MachineFunction &MF);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      MachineFunctionPass::getAnalysisUsage(AU);
    }
  };
} // end anonymous namespace

char ExpandISelPseudos::ID = 0;
char &llvm::ExpandISelPseudosID = ExpandISelPseudos::ID;
INITIALIZE_PASS(ExpandISelPseudos, "expand-isel-pseudos",
                "Expand ISel Pseudo-instructions", false, false)

bool ExpandISelPseudos::runOnMachineFunction(MachineFunction &MF) {
  bool Changed = false;
  const TargetLowering *TLI = MF.getTarget().getTargetLowering();

  // Iterate through each instruction in the function, looking for pseudos.
  for (MachineFunction::iterator I = MF.begin(), E = MF.end(); I != E; ++I) {
    MachineBasicBlock *MBB = I;
    for (MachineBasicBlock::iterator MBBI = MBB->begin(), MBBE = MBB->end();
         MBBI != MBBE; ) {
      MachineInstr *MI = MBBI++;

      // If MI is a pseudo, expand it.
      if (MI->usesCustomInsertionHook()) {
        Changed = true;
        MachineBasicBlock *NewMBB =
          TLI->EmitInstrWithCustomInserter(MI, MBB);
        // The expansion may involve new basic blocks.
        if (NewMBB != MBB) {
          MBB = NewMBB;
          I = NewMBB;
          MBBI = NewMBB->begin();
          MBBE = NewMBB->end();
        }
      }
    }
  }

  return Changed;
}