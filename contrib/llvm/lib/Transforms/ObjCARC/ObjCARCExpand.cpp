
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

#define DEBUG_TYPE "objc-arc-expand"

#include "ObjCARC.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/PassRegistry.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
  class Module;
}

using namespace llvm;
using namespace llvm::objcarc;

namespace {
  /// \brief Early ARC transformations.
  class ObjCARCExpand : public FunctionPass {
    virtual void getAnalysisUsage(AnalysisUsage &AU) const;
    virtual bool doInitialization(Module &M);
    virtual bool runOnFunction(Function &F);

    /// A flag indicating whether this optimization pass should run.
    bool Run;

  public:
    static char ID;
    ObjCARCExpand() : FunctionPass(ID) {
      initializeObjCARCExpandPass(*PassRegistry::getPassRegistry());
    }
  };
}

char ObjCARCExpand::ID = 0;
INITIALIZE_PASS(ObjCARCExpand,
                "objc-arc-expand", "ObjC ARC expansion", false, false)

Pass *llvm::createObjCARCExpandPass() {
  return new ObjCARCExpand();
}

void ObjCARCExpand::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
}

bool ObjCARCExpand::doInitialization(Module &M) {
  Run = ModuleHasARC(M);
  return false;
}

bool ObjCARCExpand::runOnFunction(Function &F) {
  if (!EnableARCOpts)
    return false;

  // If nothing in the Module uses ARC, don't do anything.
  if (!Run)
    return false;

  bool Changed = false;

  DEBUG(dbgs() << "ObjCARCExpand: Visiting Function: " << F.getName() << "\n");

  for (inst_iterator I = inst_begin(&F), E = inst_end(&F); I != E; ++I) {
    Instruction *Inst = &*I;

    DEBUG(dbgs() << "ObjCARCExpand: Visiting: " << *Inst << "\n");

    switch (GetBasicInstructionClass(Inst)) {
    case IC_Retain:
    case IC_RetainRV:
    case IC_Autorelease:
    case IC_AutoreleaseRV:
    case IC_FusedRetainAutorelease:
    case IC_FusedRetainAutoreleaseRV: {
      // These calls return their argument verbatim, as a low-level
      // optimization. However, this makes high-level optimizations
      // harder. Undo any uses of this optimization that the front-end
      // emitted here. We'll redo them in the contract pass.
      Changed = true;
      Value *Value = cast<CallInst>(Inst)->getArgOperand(0);
      DEBUG(dbgs() << "ObjCARCExpand: Old = " << *Inst << "\n"
                      "               New = " << *Value << "\n");
      Inst->replaceAllUsesWith(Value);
      break;
    }
    default:
      break;
    }
  }

  DEBUG(dbgs() << "ObjCARCExpand: Finished List.\n\n");

  return Changed;
}