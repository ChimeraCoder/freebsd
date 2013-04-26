
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
class PointerArithChecker 
  : public Checker< check::PreStmt<BinaryOperator> > {
  mutable OwningPtr<BuiltinBug> BT;

public:
  void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
};
}

void PointerArithChecker::checkPreStmt(const BinaryOperator *B,
                                       CheckerContext &C) const {
  if (B->getOpcode() != BO_Sub && B->getOpcode() != BO_Add)
    return;

  ProgramStateRef state = C.getState();
  const LocationContext *LCtx = C.getLocationContext();
  SVal LV = state->getSVal(B->getLHS(), LCtx);
  SVal RV = state->getSVal(B->getRHS(), LCtx);

  const MemRegion *LR = LV.getAsRegion();

  if (!LR || !RV.isConstant())
    return;

  // If pointer arithmetic is done on variables of non-array type, this often
  // means behavior rely on memory organization, which is dangerous.
  if (isa<VarRegion>(LR) || isa<CodeTextRegion>(LR) || 
      isa<CompoundLiteralRegion>(LR)) {

    if (ExplodedNode *N = C.addTransition()) {
      if (!BT)
        BT.reset(new BuiltinBug("Dangerous pointer arithmetic",
                            "Pointer arithmetic done on non-array variables "
                            "means reliance on memory layout, which is "
                            "dangerous."));
      BugReport *R = new BugReport(*BT, BT->getDescription(), N);
      R->addRange(B->getSourceRange());
      C.emitReport(R);
    }
  }
}

void ento::registerPointerArithChecker(CheckerManager &mgr) {
  mgr.registerChecker<PointerArithChecker>();
}