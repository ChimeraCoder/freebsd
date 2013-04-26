
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

#define DEBUG_TYPE "adce"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/InstIterator.h"
using namespace llvm;

STATISTIC(NumRemoved, "Number of instructions removed");

namespace {
  struct ADCE : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    ADCE() : FunctionPass(ID) {
      initializeADCEPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(Function& F);

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesCFG();
    }

  };
}

char ADCE::ID = 0;
INITIALIZE_PASS(ADCE, "adce", "Aggressive Dead Code Elimination", false, false)

bool ADCE::runOnFunction(Function& F) {
  SmallPtrSet<Instruction*, 128> alive;
  SmallVector<Instruction*, 128> worklist;

  // Collect the set of "root" instructions that are known live.
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    if (isa<TerminatorInst>(I.getInstructionIterator()) ||
        isa<DbgInfoIntrinsic>(I.getInstructionIterator()) ||
        isa<LandingPadInst>(I.getInstructionIterator()) ||
        I->mayHaveSideEffects()) {
      alive.insert(I.getInstructionIterator());
      worklist.push_back(I.getInstructionIterator());
    }

  // Propagate liveness backwards to operands.
  while (!worklist.empty()) {
    Instruction* curr = worklist.pop_back_val();
    for (Instruction::op_iterator OI = curr->op_begin(), OE = curr->op_end();
         OI != OE; ++OI)
      if (Instruction* Inst = dyn_cast<Instruction>(OI))
        if (alive.insert(Inst))
          worklist.push_back(Inst);
  }

  // The inverse of the live set is the dead set.  These are those instructions
  // which have no side effects and do not influence the control flow or return
  // value of the function, and may therefore be deleted safely.
  // NOTE: We reuse the worklist vector here for memory efficiency.
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    if (!alive.count(I.getInstructionIterator())) {
      worklist.push_back(I.getInstructionIterator());
      I->dropAllReferences();
    }

  for (SmallVector<Instruction*, 1024>::iterator I = worklist.begin(),
       E = worklist.end(); I != E; ++I) {
    ++NumRemoved;
    (*I)->eraseFromParent();
  }

  return !worklist.empty();
}

FunctionPass *llvm::createAggressiveDCEPass() {
  return new ADCE();
}