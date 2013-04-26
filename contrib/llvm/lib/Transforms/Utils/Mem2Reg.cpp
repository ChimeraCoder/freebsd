
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

#define DEBUG_TYPE "mem2reg"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
using namespace llvm;

STATISTIC(NumPromoted, "Number of alloca's promoted");

namespace {
  struct PromotePass : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    PromotePass() : FunctionPass(ID) {
      initializePromotePassPass(*PassRegistry::getPassRegistry());
    }

    // runOnFunction - To run this pass, first we calculate the alloca
    // instructions that are safe for promotion, then we promote each one.
    //
    virtual bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DominatorTree>();
      AU.setPreservesCFG();
      // This is a cluster of orthogonal Transforms
      AU.addPreserved<UnifyFunctionExitNodes>();
      AU.addPreservedID(LowerSwitchID);
      AU.addPreservedID(LowerInvokePassID);
    }
  };
}  // end of anonymous namespace

char PromotePass::ID = 0;
INITIALIZE_PASS_BEGIN(PromotePass, "mem2reg", "Promote Memory to Register",
                false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTree)
INITIALIZE_PASS_END(PromotePass, "mem2reg", "Promote Memory to Register",
                false, false)

bool PromotePass::runOnFunction(Function &F) {
  std::vector<AllocaInst*> Allocas;

  BasicBlock &BB = F.getEntryBlock();  // Get the entry node for the function

  bool Changed  = false;

  DominatorTree &DT = getAnalysis<DominatorTree>();

  while (1) {
    Allocas.clear();

    // Find allocas that are safe to promote, by looking at all instructions in
    // the entry node
    for (BasicBlock::iterator I = BB.begin(), E = --BB.end(); I != E; ++I)
      if (AllocaInst *AI = dyn_cast<AllocaInst>(I))       // Is it an alloca?
        if (isAllocaPromotable(AI))
          Allocas.push_back(AI);

    if (Allocas.empty()) break;

    PromoteMemToReg(Allocas, DT);
    NumPromoted += Allocas.size();
    Changed = true;
  }

  return Changed;
}

// createPromoteMemoryToRegister - Provide an entry point to create this pass.
//
FunctionPass *llvm::createPromoteMemoryToRegisterPass() {
  return new PromotePass();
}