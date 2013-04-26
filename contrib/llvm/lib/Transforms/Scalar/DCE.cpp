
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

#define DEBUG_TYPE "dce"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Transforms/Utils/Local.h"
using namespace llvm;

STATISTIC(DIEEliminated, "Number of insts removed by DIE pass");
STATISTIC(DCEEliminated, "Number of insts removed");

namespace {
  //===--------------------------------------------------------------------===//
  // DeadInstElimination pass implementation
  //
  struct DeadInstElimination : public BasicBlockPass {
    static char ID; // Pass identification, replacement for typeid
    DeadInstElimination() : BasicBlockPass(ID) {
      initializeDeadInstEliminationPass(*PassRegistry::getPassRegistry());
    }
    virtual bool runOnBasicBlock(BasicBlock &BB) {
      TargetLibraryInfo *TLI = getAnalysisIfAvailable<TargetLibraryInfo>();
      bool Changed = false;
      for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
        Instruction *Inst = DI++;
        if (isInstructionTriviallyDead(Inst, TLI)) {
          Inst->eraseFromParent();
          Changed = true;
          ++DIEEliminated;
        }
      }
      return Changed;
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
    }
  };
}

char DeadInstElimination::ID = 0;
INITIALIZE_PASS(DeadInstElimination, "die",
                "Dead Instruction Elimination", false, false)

Pass *llvm::createDeadInstEliminationPass() {
  return new DeadInstElimination();
}


namespace {
  //===--------------------------------------------------------------------===//
  // DeadCodeElimination pass implementation
  //
  struct DCE : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    DCE() : FunctionPass(ID) {
      initializeDCEPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(Function &F);

     virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
    }
 };
}

char DCE::ID = 0;
INITIALIZE_PASS(DCE, "dce", "Dead Code Elimination", false, false)

bool DCE::runOnFunction(Function &F) {
  TargetLibraryInfo *TLI = getAnalysisIfAvailable<TargetLibraryInfo>();

  // Start out with all of the instructions in the worklist...
  std::vector<Instruction*> WorkList;
  for (inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i)
    WorkList.push_back(&*i);

  // Loop over the worklist finding instructions that are dead.  If they are
  // dead make them drop all of their uses, making other instructions
  // potentially dead, and work until the worklist is empty.
  //
  bool MadeChange = false;
  while (!WorkList.empty()) {
    Instruction *I = WorkList.back();
    WorkList.pop_back();

    if (isInstructionTriviallyDead(I, TLI)) { // If the instruction is dead.
      // Loop over all of the values that the instruction uses, if there are
      // instructions being used, add them to the worklist, because they might
      // go dead after this one is removed.
      //
      for (User::op_iterator OI = I->op_begin(), E = I->op_end(); OI != E; ++OI)
        if (Instruction *Used = dyn_cast<Instruction>(*OI))
          WorkList.push_back(Used);

      // Remove the instruction.
      I->eraseFromParent();

      // Remove the instruction from the worklist if it still exists in it.
      WorkList.erase(std::remove(WorkList.begin(), WorkList.end(), I),
                     WorkList.end());

      MadeChange = true;
      ++DCEEliminated;
    }
  }
  return MadeChange;
}

FunctionPass *llvm::createDeadCodeEliminationPass() {
  return new DCE();
}