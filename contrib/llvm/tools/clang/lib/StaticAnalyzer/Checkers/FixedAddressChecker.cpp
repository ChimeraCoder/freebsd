
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
class FixedAddressChecker 
  : public Checker< check::PreStmt<BinaryOperator> > {
  mutable OwningPtr<BuiltinBug> BT;

public:
  void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
};
}

void FixedAddressChecker::checkPreStmt(const BinaryOperator *B,
                                       CheckerContext &C) const {
  // Using a fixed address is not portable because that address will probably
  // not be valid in all environments or platforms.

  if (B->getOpcode() != BO_Assign)
    return;

  QualType T = B->getType();
  if (!T->isPointerType())
    return;

  ProgramStateRef state = C.getState();
  SVal RV = state->getSVal(B->getRHS(), C.getLocationContext());

  if (!RV.isConstant() || RV.isZeroConstant())
    return;

  if (ExplodedNode *N = C.addTransition()) {
    if (!BT)
      BT.reset(new BuiltinBug("Use fixed address", 
                          "Using a fixed address is not portable because that "
                          "address will probably not be valid in all "
                          "environments or platforms."));
    BugReport *R = new BugReport(*BT, BT->getDescription(), N);
    R->addRange(B->getRHS()->getSourceRange());
    C.emitReport(R);
  }
}

void ento::registerFixedAddressChecker(CheckerManager &mgr) {
  mgr.registerChecker<FixedAddressChecker>();
}