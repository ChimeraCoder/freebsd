
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

#define DEBUG_TYPE "inline"
#include "llvm/Transforms/IPO.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Transforms/IPO/InlinerPass.h"

using namespace llvm;

namespace {

/// \brief Actaul inliner pass implementation.
///
/// The common implementation of the inlining logic is shared between this
/// inliner pass and the always inliner pass. The two passes use different cost
/// analyses to determine when to inline.
class SimpleInliner : public Inliner {
  InlineCostAnalysis *ICA;

public:
  SimpleInliner() : Inliner(ID), ICA(0) {
    initializeSimpleInlinerPass(*PassRegistry::getPassRegistry());
  }

  SimpleInliner(int Threshold)
      : Inliner(ID, Threshold, /*InsertLifetime*/ true), ICA(0) {
    initializeSimpleInlinerPass(*PassRegistry::getPassRegistry());
  }

  static char ID; // Pass identification, replacement for typeid

  InlineCost getInlineCost(CallSite CS) {
    return ICA->getInlineCost(CS, getInlineThreshold(CS));
  }

  virtual bool runOnSCC(CallGraphSCC &SCC);
  virtual void getAnalysisUsage(AnalysisUsage &AU) const;
};

} // end anonymous namespace

char SimpleInliner::ID = 0;
INITIALIZE_PASS_BEGIN(SimpleInliner, "inline",
                "Function Integration/Inlining", false, false)
INITIALIZE_AG_DEPENDENCY(CallGraph)
INITIALIZE_PASS_DEPENDENCY(InlineCostAnalysis)
INITIALIZE_PASS_END(SimpleInliner, "inline",
                "Function Integration/Inlining", false, false)

Pass *llvm::createFunctionInliningPass() { return new SimpleInliner(); }

Pass *llvm::createFunctionInliningPass(int Threshold) {
  return new SimpleInliner(Threshold);
}

bool SimpleInliner::runOnSCC(CallGraphSCC &SCC) {
  ICA = &getAnalysis<InlineCostAnalysis>();
  return Inliner::runOnSCC(SCC);
}

void SimpleInliner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<InlineCostAnalysis>();
  Inliner::getAnalysisUsage(AU);
}