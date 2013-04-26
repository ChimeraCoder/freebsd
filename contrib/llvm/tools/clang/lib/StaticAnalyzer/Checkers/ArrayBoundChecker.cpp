
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
class ArrayBoundChecker : 
    public Checker<check::Location> {
  mutable OwningPtr<BuiltinBug> BT;
public:
  void checkLocation(SVal l, bool isLoad, const Stmt* S,
                     CheckerContext &C) const;
};
}

void ArrayBoundChecker::checkLocation(SVal l, bool isLoad, const Stmt* LoadS,
                                      CheckerContext &C) const {
  // Check for out of bound array element access.
  const MemRegion *R = l.getAsRegion();
  if (!R)
    return;

  const ElementRegion *ER = dyn_cast<ElementRegion>(R);
  if (!ER)
    return;

  // Get the index of the accessed element.
  DefinedOrUnknownSVal Idx = ER->getIndex().castAs<DefinedOrUnknownSVal>();

  // Zero index is always in bound, this also passes ElementRegions created for
  // pointer casts.
  if (Idx.isZeroConstant())
    return;

  ProgramStateRef state = C.getState();

  // Get the size of the array.
  DefinedOrUnknownSVal NumElements 
    = C.getStoreManager().getSizeInElements(state, ER->getSuperRegion(), 
                                            ER->getValueType());

  ProgramStateRef StInBound = state->assumeInBound(Idx, NumElements, true);
  ProgramStateRef StOutBound = state->assumeInBound(Idx, NumElements, false);
  if (StOutBound && !StInBound) {
    ExplodedNode *N = C.generateSink(StOutBound);
    if (!N)
      return;
  
    if (!BT)
      BT.reset(new BuiltinBug("Out-of-bound array access",
                       "Access out-of-bound array element (buffer overflow)"));

    // FIXME: It would be nice to eventually make this diagnostic more clear,
    // e.g., by referencing the original declaration or by saying *why* this
    // reference is outside the range.

    // Generate a report for this bug.
    BugReport *report = 
      new BugReport(*BT, BT->getDescription(), N);

    report->addRange(LoadS->getSourceRange());
    C.emitReport(report);
    return;
  }
  
  // Array bound check succeeded.  From this point forward the array bound
  // should always succeed.
  C.addTransition(StInBound);
}

void ento::registerArrayBoundChecker(CheckerManager &mgr) {
  mgr.registerChecker<ArrayBoundChecker>();
}