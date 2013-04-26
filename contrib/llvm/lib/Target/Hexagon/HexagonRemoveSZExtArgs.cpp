
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

#include "Hexagon.h"
#include "HexagonTargetMachine.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;
namespace {
  struct HexagonRemoveExtendArgs : public FunctionPass {
  public:
    static char ID;
    HexagonRemoveExtendArgs() : FunctionPass(ID) {}
    virtual bool runOnFunction(Function &F);

    const char *getPassName() const {
      return "Remove sign extends";
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<MachineFunctionAnalysis>();
      AU.addPreserved<MachineFunctionAnalysis>();
      FunctionPass::getAnalysisUsage(AU);
    }
  };
}

char HexagonRemoveExtendArgs::ID = 0;
RegisterPass<HexagonRemoveExtendArgs> X("reargs",
                                        "Remove Sign and Zero Extends for Args"
                                        );



bool HexagonRemoveExtendArgs::runOnFunction(Function &F) {
  unsigned Idx = 1;
  for (Function::arg_iterator AI = F.arg_begin(), AE = F.arg_end(); AI != AE;
       ++AI, ++Idx) {
    if (F.getAttributes().hasAttribute(Idx, Attribute::SExt)) {
      Argument* Arg = AI;
      if (!isa<PointerType>(Arg->getType())) {
        for (Instruction::use_iterator UI = Arg->use_begin();
             UI != Arg->use_end();) {
          if (isa<SExtInst>(*UI)) {
            Instruction* Use = cast<Instruction>(*UI);
            SExtInst* SI = new SExtInst(Arg, Use->getType());
            assert (EVT::getEVT(SI->getType()) ==
                    (EVT::getEVT(Use->getType())));
            ++UI;
            Use->replaceAllUsesWith(SI);
            Instruction* First = F.getEntryBlock().begin();
            SI->insertBefore(First);
            Use->eraseFromParent();
          } else {
            ++UI;
          }
        }
      }
    }
  }
  return true;
}



FunctionPass *llvm::createHexagonRemoveExtendOps(HexagonTargetMachine &TM) {
  return new HexagonRemoveExtendArgs();
}