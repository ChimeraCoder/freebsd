
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
#include "clang/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"

using namespace clang;
using namespace ento;

namespace {
class ReturnPointerRangeChecker : 
    public Checker< check::PreStmt<ReturnStmt> > {
  mutable OwningPtr<BuiltinBug> BT;
public:
    void checkPreStmt(const ReturnStmt *RS, CheckerContext &C) const;
};
}

void ReturnPointerRangeChecker::checkPreStmt(const ReturnStmt *RS,
                                             CheckerContext &C) const {
  ProgramStateRef state = C.getState();

  const Expr *RetE = RS->getRetValue();
  if (!RetE)
    return;
 
  SVal V = state->getSVal(RetE, C.getLocationContext());
  const MemRegion *R = V.getAsRegion();

  const ElementRegion *ER = dyn_cast_or_null<ElementRegion>(R);
  if (!ER)
    return;

  DefinedOrUnknownSVal Idx = ER->getIndex().castAs<DefinedOrUnknownSVal>();
  // Zero index is always in bound, this also passes ElementRegions created for
  // pointer casts.
  if (Idx.isZeroConstant())
    return;
  // FIXME: All of this out-of-bounds checking should eventually be refactored
  // into a common place.

  DefinedOrUnknownSVal NumElements
    = C.getStoreManager().getSizeInElements(state, ER->getSuperRegion(),
                                           ER->getValueType());

  ProgramStateRef StInBound = state->assumeInBound(Idx, NumElements, true);
  ProgramStateRef StOutBound = state->assumeInBound(Idx, NumElements, false);
  if (StOutBound && !StInBound) {
    ExplodedNode *N = C.generateSink(StOutBound);

    if (!N)
      return;
  
    // FIXME: This bug correspond to CWE-466.  Eventually we should have bug
    // types explicitly reference such exploit categories (when applicable).
    if (!BT)
      BT.reset(new BuiltinBug("Return of pointer value outside of expected range",
           "Returned pointer value points outside the original object "
           "(potential buffer overflow)"));

    // FIXME: It would be nice to eventually make this diagnostic more clear,
    // e.g., by referencing the original declaration or by saying *why* this
    // reference is outside the range.

    // Generate a report for this bug.
    BugReport *report = 
      new BugReport(*BT, BT->getDescription(), N);

    report->addRange(RetE->getSourceRange());
    C.emitReport(report);
  }
}

void ento::registerReturnPointerRangeChecker(CheckerManager &mgr) {
  mgr.registerChecker<ReturnPointerRangeChecker>();
}