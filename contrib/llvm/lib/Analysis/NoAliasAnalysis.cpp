
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
#include "llvm/IR/DataLayout.h"
#include "llvm/Pass.h"
using namespace llvm;

namespace {
  /// NoAA - This class implements the -no-aa pass, which always returns "I
  /// don't know" for alias queries.  NoAA is unlike other alias analysis
  /// implementations, in that it does not chain to a previous analysis.  As
  /// such it doesn't follow many of the rules that other alias analyses must.
  ///
  struct NoAA : public ImmutablePass, public AliasAnalysis {
    static char ID; // Class identification, replacement for typeinfo
    NoAA() : ImmutablePass(ID) {
      initializeNoAAPass(*PassRegistry::getPassRegistry());
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    }

    virtual void initializePass() {
      // Note: NoAA does not call InitializeAliasAnalysis because it's
      // special and does not support chaining.
      TD = getAnalysisIfAvailable<DataLayout>();
    }

    virtual AliasResult alias(const Location &LocA, const Location &LocB) {
      return MayAlias;
    }

    virtual ModRefBehavior getModRefBehavior(ImmutableCallSite CS) {
      return UnknownModRefBehavior;
    }
    virtual ModRefBehavior getModRefBehavior(const Function *F) {
      return UnknownModRefBehavior;
    }

    virtual bool pointsToConstantMemory(const Location &Loc,
                                        bool OrLocal) {
      return false;
    }
    virtual ModRefResult getModRefInfo(ImmutableCallSite CS,
                                       const Location &Loc) {
      return ModRef;
    }
    virtual ModRefResult getModRefInfo(ImmutableCallSite CS1,
                                       ImmutableCallSite CS2) {
      return ModRef;
    }

    virtual void deleteValue(Value *V) {}
    virtual void copyValue(Value *From, Value *To) {}
    virtual void addEscapingUse(Use &U) {}
    
    /// getAdjustedAnalysisPointer - This method is used when a pass implements
    /// an analysis interface through multiple inheritance.  If needed, it
    /// should override this to adjust the this pointer as needed for the
    /// specified pass info.
    virtual void *getAdjustedAnalysisPointer(const void *ID) {
      if (ID == &AliasAnalysis::ID)
        return (AliasAnalysis*)this;
      return this;
    }
  };
}  // End of anonymous namespace

// Register this pass...
char NoAA::ID = 0;
INITIALIZE_AG_PASS(NoAA, AliasAnalysis, "no-aa",
                   "No Alias Analysis (always returns 'may' alias)",
                   true, true, true)

ImmutablePass *llvm::createNoAAPass() { return new NoAA(); }