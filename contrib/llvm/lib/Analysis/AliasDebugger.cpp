
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

#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include <set>
using namespace llvm;

namespace {
  
  class AliasDebugger : public ModulePass, public AliasAnalysis {

    //What we do is simple.  Keep track of every value the AA could
    //know about, and verify that queries are one of those.
    //A query to a value that didn't exist when the AA was created
    //means someone forgot to update the AA when creating new values

    std::set<const Value*> Vals;
    
  public:
    static char ID; // Class identification, replacement for typeinfo
    AliasDebugger() : ModulePass(ID) {
      initializeAliasDebuggerPass(*PassRegistry::getPassRegistry());
    }

    bool runOnModule(Module &M) {
      InitializeAliasAnalysis(this);                 // set up super class

      for(Module::global_iterator I = M.global_begin(),
            E = M.global_end(); I != E; ++I) {
        Vals.insert(&*I);
        for (User::const_op_iterator OI = I->op_begin(),
             OE = I->op_end(); OI != OE; ++OI)
          Vals.insert(*OI);
      }

      for(Module::iterator I = M.begin(),
            E = M.end(); I != E; ++I){
        Vals.insert(&*I);
        if(!I->isDeclaration()) {
          for (Function::arg_iterator AI = I->arg_begin(), AE = I->arg_end();
               AI != AE; ++AI) 
            Vals.insert(&*AI);     
          for (Function::const_iterator FI = I->begin(), FE = I->end();
               FI != FE; ++FI) 
            for (BasicBlock::const_iterator BI = FI->begin(), BE = FI->end();
                 BI != BE; ++BI) {
              Vals.insert(&*BI);
              for (User::const_op_iterator OI = BI->op_begin(),
                   OE = BI->op_end(); OI != OE; ++OI)
                Vals.insert(*OI);
            }
        }
        
      }
      return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AliasAnalysis::getAnalysisUsage(AU);
      AU.setPreservesAll();                         // Does not transform code
    }

    /// getAdjustedAnalysisPointer - This method is used when a pass implements
    /// an analysis interface through multiple inheritance.  If needed, it
    /// should override this to adjust the this pointer as needed for the
    /// specified pass info.
    virtual void *getAdjustedAnalysisPointer(AnalysisID PI) {
      if (PI == &AliasAnalysis::ID)
        return (AliasAnalysis*)this;
      return this;
    }
    
    //------------------------------------------------
    // Implement the AliasAnalysis API
    //
    AliasResult alias(const Location &LocA, const Location &LocB) {
      assert(Vals.find(LocA.Ptr) != Vals.end() &&
             "Never seen value in AA before");
      assert(Vals.find(LocB.Ptr) != Vals.end() &&
             "Never seen value in AA before");
      return AliasAnalysis::alias(LocA, LocB);
    }

    ModRefResult getModRefInfo(ImmutableCallSite CS,
                               const Location &Loc) {
      assert(Vals.find(Loc.Ptr) != Vals.end() && "Never seen value in AA before");
      return AliasAnalysis::getModRefInfo(CS, Loc);
    }

    ModRefResult getModRefInfo(ImmutableCallSite CS1,
                               ImmutableCallSite CS2) {
      return AliasAnalysis::getModRefInfo(CS1,CS2);
    }
    
    bool pointsToConstantMemory(const Location &Loc, bool OrLocal) {
      assert(Vals.find(Loc.Ptr) != Vals.end() && "Never seen value in AA before");
      return AliasAnalysis::pointsToConstantMemory(Loc, OrLocal);
    }

    virtual void deleteValue(Value *V) {
      assert(Vals.find(V) != Vals.end() && "Never seen value in AA before");
      AliasAnalysis::deleteValue(V);
    }
    virtual void copyValue(Value *From, Value *To) {
      Vals.insert(To);
      AliasAnalysis::copyValue(From, To);
    }

  };
}

char AliasDebugger::ID = 0;
INITIALIZE_AG_PASS(AliasDebugger, AliasAnalysis, "debug-aa",
                   "AA use debugger", false, true, false)

Pass *llvm::createAliasDebugger() { return new AliasDebugger(); }