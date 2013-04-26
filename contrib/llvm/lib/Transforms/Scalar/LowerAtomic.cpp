
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

#define DEBUG_TYPE "loweratomic"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
using namespace llvm;

static bool LowerAtomicCmpXchgInst(AtomicCmpXchgInst *CXI) {
  IRBuilder<> Builder(CXI->getParent(), CXI);
  Value *Ptr = CXI->getPointerOperand();
  Value *Cmp = CXI->getCompareOperand();
  Value *Val = CXI->getNewValOperand();

  LoadInst *Orig = Builder.CreateLoad(Ptr);
  Value *Equal = Builder.CreateICmpEQ(Orig, Cmp);
  Value *Res = Builder.CreateSelect(Equal, Val, Orig);
  Builder.CreateStore(Res, Ptr);

  CXI->replaceAllUsesWith(Orig);
  CXI->eraseFromParent();
  return true;
}

static bool LowerAtomicRMWInst(AtomicRMWInst *RMWI) {
  IRBuilder<> Builder(RMWI->getParent(), RMWI);
  Value *Ptr = RMWI->getPointerOperand();
  Value *Val = RMWI->getValOperand();

  LoadInst *Orig = Builder.CreateLoad(Ptr);
  Value *Res = NULL;

  switch (RMWI->getOperation()) {
  default: llvm_unreachable("Unexpected RMW operation");
  case AtomicRMWInst::Xchg:
    Res = Val;
    break;
  case AtomicRMWInst::Add:
    Res = Builder.CreateAdd(Orig, Val);
    break;
  case AtomicRMWInst::Sub:
    Res = Builder.CreateSub(Orig, Val);
    break;
  case AtomicRMWInst::And:
    Res = Builder.CreateAnd(Orig, Val);
    break;
  case AtomicRMWInst::Nand:
    Res = Builder.CreateNot(Builder.CreateAnd(Orig, Val));
    break;
  case AtomicRMWInst::Or:
    Res = Builder.CreateOr(Orig, Val);
    break;
  case AtomicRMWInst::Xor:
    Res = Builder.CreateXor(Orig, Val);
    break;
  case AtomicRMWInst::Max:
    Res = Builder.CreateSelect(Builder.CreateICmpSLT(Orig, Val),
                               Val, Orig);
    break;
  case AtomicRMWInst::Min:
    Res = Builder.CreateSelect(Builder.CreateICmpSLT(Orig, Val),
                               Orig, Val);
    break;
  case AtomicRMWInst::UMax:
    Res = Builder.CreateSelect(Builder.CreateICmpULT(Orig, Val),
                               Val, Orig);
    break;
  case AtomicRMWInst::UMin:
    Res = Builder.CreateSelect(Builder.CreateICmpULT(Orig, Val),
                               Orig, Val);
    break;
  }
  Builder.CreateStore(Res, Ptr);
  RMWI->replaceAllUsesWith(Orig);
  RMWI->eraseFromParent();
  return true;
}

static bool LowerFenceInst(FenceInst *FI) {
  FI->eraseFromParent();
  return true;
}

static bool LowerLoadInst(LoadInst *LI) {
  LI->setAtomic(NotAtomic);
  return true;
}

static bool LowerStoreInst(StoreInst *SI) {
  SI->setAtomic(NotAtomic);
  return true;
}

namespace {
  struct LowerAtomic : public BasicBlockPass {
    static char ID;
    LowerAtomic() : BasicBlockPass(ID) {
      initializeLowerAtomicPass(*PassRegistry::getPassRegistry());
    }
    bool runOnBasicBlock(BasicBlock &BB) {
      bool Changed = false;
      for (BasicBlock::iterator DI = BB.begin(), DE = BB.end(); DI != DE; ) {
        Instruction *Inst = DI++;
        if (FenceInst *FI = dyn_cast<FenceInst>(Inst))
          Changed |= LowerFenceInst(FI);
        else if (AtomicCmpXchgInst *CXI = dyn_cast<AtomicCmpXchgInst>(Inst))
          Changed |= LowerAtomicCmpXchgInst(CXI);
        else if (AtomicRMWInst *RMWI = dyn_cast<AtomicRMWInst>(Inst))
          Changed |= LowerAtomicRMWInst(RMWI);
        else if (LoadInst *LI = dyn_cast<LoadInst>(Inst)) {
          if (LI->isAtomic())
            LowerLoadInst(LI);
        } else if (StoreInst *SI = dyn_cast<StoreInst>(Inst)) {
          if (SI->isAtomic())
            LowerStoreInst(SI);
        }
      }
      return Changed;
    }
  };
}

char LowerAtomic::ID = 0;
INITIALIZE_PASS(LowerAtomic, "loweratomic",
                "Lower atomic intrinsics to non-atomic form",
                false, false)

Pass *llvm::createLowerAtomicPass() { return new LowerAtomic(); }