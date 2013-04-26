
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
#include "llvm/Assembly/Writer.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

static cl::opt<bool>
PrintAll("count-aa-print-all-queries", cl::ReallyHidden, cl::init(true));
static cl::opt<bool>
PrintAllFailures("count-aa-print-all-failed-queries", cl::ReallyHidden);

namespace {
  class AliasAnalysisCounter : public ModulePass, public AliasAnalysis {
    unsigned No, May, Partial, Must;
    unsigned NoMR, JustRef, JustMod, MR;
    Module *M;
  public:
    static char ID; // Class identification, replacement for typeinfo
    AliasAnalysisCounter() : ModulePass(ID) {
      initializeAliasAnalysisCounterPass(*PassRegistry::getPassRegistry());
      No = May = Partial = Must = 0;
      NoMR = JustRef = JustMod = MR = 0;
    }

    void printLine(const char *Desc, unsigned Val, unsigned Sum) {
      errs() <<  "  " << Val << " " << Desc << " responses ("
             << Val*100/Sum << "%)\n";
    }
    ~AliasAnalysisCounter() {
      unsigned AASum = No+May+Partial+Must;
      unsigned MRSum = NoMR+JustRef+JustMod+MR;
      if (AASum + MRSum) { // Print a report if any counted queries occurred...
        errs() << "\n===== Alias Analysis Counter Report =====\n"
               << "  Analysis counted:\n"
               << "  " << AASum << " Total Alias Queries Performed\n";
        if (AASum) {
          printLine("no alias",     No, AASum);
          printLine("may alias",   May, AASum);
          printLine("partial alias", Partial, AASum);
          printLine("must alias", Must, AASum);
          errs() << "  Alias Analysis Counter Summary: " << No*100/AASum << "%/"
                 << May*100/AASum << "%/"
                 << Partial*100/AASum << "%/"
                 << Must*100/AASum<<"%\n\n";
        }

        errs() << "  " << MRSum    << " Total Mod/Ref Queries Performed\n";
        if (MRSum) {
          printLine("no mod/ref",    NoMR, MRSum);
          printLine("ref",        JustRef, MRSum);
          printLine("mod",        JustMod, MRSum);
          printLine("mod/ref",         MR, MRSum);
          errs() << "  Mod/Ref Analysis Counter Summary: " <<NoMR*100/MRSum
                 << "%/" << JustRef*100/MRSum << "%/" << JustMod*100/MRSum
                 << "%/" << MR*100/MRSum <<"%\n\n";
        }
      }
    }

    bool runOnModule(Module &M) {
      this->M = &M;
      InitializeAliasAnalysis(this);
      return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AliasAnalysis::getAnalysisUsage(AU);
      AU.addRequired<AliasAnalysis>();
      AU.setPreservesAll();
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
    
    // FIXME: We could count these too...
    bool pointsToConstantMemory(const Location &Loc, bool OrLocal) {
      return getAnalysis<AliasAnalysis>().pointsToConstantMemory(Loc, OrLocal);
    }

    // Forwarding functions: just delegate to a real AA implementation, counting
    // the number of responses...
    AliasResult alias(const Location &LocA, const Location &LocB);

    ModRefResult getModRefInfo(ImmutableCallSite CS,
                               const Location &Loc);
    ModRefResult getModRefInfo(ImmutableCallSite CS1,
                               ImmutableCallSite CS2) {
      return AliasAnalysis::getModRefInfo(CS1,CS2);
    }
  };
}

char AliasAnalysisCounter::ID = 0;
INITIALIZE_AG_PASS(AliasAnalysisCounter, AliasAnalysis, "count-aa",
                   "Count Alias Analysis Query Responses", false, true, false)

ModulePass *llvm::createAliasAnalysisCounterPass() {
  return new AliasAnalysisCounter();
}

AliasAnalysis::AliasResult
AliasAnalysisCounter::alias(const Location &LocA, const Location &LocB) {
  AliasResult R = getAnalysis<AliasAnalysis>().alias(LocA, LocB);

  const char *AliasString = 0;
  switch (R) {
  case NoAlias:   No++;   AliasString = "No alias"; break;
  case MayAlias:  May++;  AliasString = "May alias"; break;
  case PartialAlias: Partial++; AliasString = "Partial alias"; break;
  case MustAlias: Must++; AliasString = "Must alias"; break;
  }

  if (PrintAll || (PrintAllFailures && R == MayAlias)) {
    errs() << AliasString << ":\t";
    errs() << "[" << LocA.Size << "B] ";
    WriteAsOperand(errs(), LocA.Ptr, true, M);
    errs() << ", ";
    errs() << "[" << LocB.Size << "B] ";
    WriteAsOperand(errs(), LocB.Ptr, true, M);
    errs() << "\n";
  }

  return R;
}

AliasAnalysis::ModRefResult
AliasAnalysisCounter::getModRefInfo(ImmutableCallSite CS,
                                    const Location &Loc) {
  ModRefResult R = getAnalysis<AliasAnalysis>().getModRefInfo(CS, Loc);

  const char *MRString = 0;
  switch (R) {
  case NoModRef: NoMR++;     MRString = "NoModRef"; break;
  case Ref:      JustRef++;  MRString = "JustRef"; break;
  case Mod:      JustMod++;  MRString = "JustMod"; break;
  case ModRef:   MR++;       MRString = "ModRef"; break;
  }

  if (PrintAll || (PrintAllFailures && R == ModRef)) {
    errs() << MRString << ":  Ptr: ";
    errs() << "[" << Loc.Size << "B] ";
    WriteAsOperand(errs(), Loc.Ptr, true, M);
    errs() << "\t<->" << *CS.getInstruction() << '\n';
  }
  return R;
}