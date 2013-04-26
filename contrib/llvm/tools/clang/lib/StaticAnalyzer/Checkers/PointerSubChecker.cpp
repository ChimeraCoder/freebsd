
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

#include "ClangSACheckers.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {
class PointerSubChecker 
  : public Checker< check::PreStmt<BinaryOperator> > {
  mutable OwningPtr<BuiltinBug> BT;

public:
  void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
};
}

void PointerSubChecker::checkPreStmt(const BinaryOperator *B,
                                     CheckerContext &C) const {
  // When doing pointer subtraction, if the two pointers do not point to the
  // same memory chunk, emit a warning.
  if (B->getOpcode() != BO_Sub)
    return;

  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();
  SVal LV = state->getSVal(B->getLHS(), LCtx);
  SVal RV = state->getSVal(B->getRHS(), LCtx);

  const MemRegion *LR = LV.getAsRegion();
  const MemRegion *RR = RV.getAsRegion();

  if (!(LR && RR))
    return;

  const MemRegion *BaseLR = LR->getBaseRegion();
  const MemRegion *BaseRR = RR->getBaseRegion();

  if (BaseLR == BaseRR)
    return;

  // Allow arithmetic on different symbolic regions.
  if (isa<SymbolicRegion>(BaseLR) || isa<SymbolicRegion>(BaseRR))
    return;

  if (ExplodedNode *N = C.addTransition()) {
    if (!BT)
      BT.reset(new BuiltinBug("Pointer subtraction", 
                          "Subtraction of two pointers that do not point to "
                          "the same memory chunk may cause incorrect result."));
    BugReport *R = new BugReport(*BT, BT->getDescription(), N);
    R->addRange(B->getSourceRange());
    C.emitReport(R);
  }
}

void ento::registerPointerSubChecker(CheckerManager &mgr) {
  mgr.registerChecker<PointerSubChecker>();
}