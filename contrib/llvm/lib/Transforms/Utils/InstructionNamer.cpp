
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

#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
using namespace llvm;

namespace {
  struct InstNamer : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    InstNamer() : FunctionPass(ID) {
      initializeInstNamerPass(*PassRegistry::getPassRegistry());
    }
    
    void getAnalysisUsage(AnalysisUsage &Info) const {
      Info.setPreservesAll();
    }

    bool runOnFunction(Function &F) {
      for (Function::arg_iterator AI = F.arg_begin(), AE = F.arg_end();
           AI != AE; ++AI)
        if (!AI->hasName() && !AI->getType()->isVoidTy())
          AI->setName("arg");

      for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
        if (!BB->hasName())
          BB->setName("bb");
        
        for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I)
          if (!I->hasName() && !I->getType()->isVoidTy())
            I->setName("tmp");
      }
      return true;
    }
  };
  
  char InstNamer::ID = 0;
}

INITIALIZE_PASS(InstNamer, "instnamer", 
                "Assign names to anonymous instructions", false, false)
char &llvm::InstructionNamerID = InstNamer::ID;
//===----------------------------------------------------------------------===//
//
// InstructionNamer - Give any unnamed non-void instructions "tmp" names.
//
FunctionPass *llvm::createInstructionNamerPass() {
  return new InstNamer();
}