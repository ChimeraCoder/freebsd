
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

#define DEBUG_TYPE "instcount"
#include "llvm/Analysis/Passes.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/InstVisitor.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

STATISTIC(TotalInsts , "Number of instructions (of all types)");
STATISTIC(TotalBlocks, "Number of basic blocks");
STATISTIC(TotalFuncs , "Number of non-external functions");
STATISTIC(TotalMemInst, "Number of memory instructions");

#define HANDLE_INST(N, OPCODE, CLASS) \
  STATISTIC(Num ## OPCODE ## Inst, "Number of " #OPCODE " insts");

#include "llvm/IR/Instruction.def"


namespace {
  class InstCount : public FunctionPass, public InstVisitor<InstCount> {
    friend class InstVisitor<InstCount>;

    void visitFunction  (Function &F) { ++TotalFuncs; }
    void visitBasicBlock(BasicBlock &BB) { ++TotalBlocks; }

#define HANDLE_INST(N, OPCODE, CLASS) \
    void visit##OPCODE(CLASS &) { ++Num##OPCODE##Inst; ++TotalInsts; }

#include "llvm/IR/Instruction.def"

    void visitInstruction(Instruction &I) {
      errs() << "Instruction Count does not know about " << I;
      llvm_unreachable(0);
    }
  public:
    static char ID; // Pass identification, replacement for typeid
    InstCount() : FunctionPass(ID) {
      initializeInstCountPass(*PassRegistry::getPassRegistry());
    }

    virtual bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    }
    virtual void print(raw_ostream &O, const Module *M) const {}

  };
}

char InstCount::ID = 0;
INITIALIZE_PASS(InstCount, "instcount",
                "Counts the various types of Instructions", false, true)

FunctionPass *llvm::createInstCountPass() { return new InstCount(); }

// InstCount::run - This is the main Analysis entry point for a
// function.
//
bool InstCount::runOnFunction(Function &F) {
  unsigned StartMemInsts =
    NumGetElementPtrInst + NumLoadInst + NumStoreInst + NumCallInst +
    NumInvokeInst + NumAllocaInst;
  visit(F);
  unsigned EndMemInsts =
    NumGetElementPtrInst + NumLoadInst + NumStoreInst + NumCallInst +
    NumInvokeInst + NumAllocaInst;
  TotalMemInst += EndMemInsts-StartMemInsts;
  return false;
}