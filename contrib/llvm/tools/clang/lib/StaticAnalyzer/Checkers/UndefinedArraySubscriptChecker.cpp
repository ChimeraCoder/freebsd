
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
class UndefinedArraySubscriptChecker
  : public Checker< check::PreStmt<ArraySubscriptExpr> > {
  mutable OwningPtr<BugType> BT;

public:
  void checkPreStmt(const ArraySubscriptExpr *A, CheckerContext &C) const;
};
} // end anonymous namespace

void 
UndefinedArraySubscriptChecker::checkPreStmt(const ArraySubscriptExpr *A,
                                             CheckerContext &C) const {
  const Expr *Index = A->getIdx();
  if (!C.getSVal(Index).isUndef())
    return;

  // Sema generates anonymous array variables for copying array struct fields.
  // Don't warn if we're in an implicitly-generated constructor.
  const Decl *D = C.getLocationContext()->getDecl();
  if (const CXXConstructorDecl *Ctor = dyn_cast<CXXConstructorDecl>(D))
    if (Ctor->isImplicitlyDefined())
      return;

  ExplodedNode *N = C.generateSink();
  if (!N)
    return;
  if (!BT)
    BT.reset(new BuiltinBug("Array subscript is undefined"));

  // Generate a report for this bug.
  BugReport *R = new BugReport(*BT, BT->getName(), N);
  R->addRange(A->getIdx()->getSourceRange());
  bugreporter::trackNullOrUndefValue(N, A->getIdx(), *R);
  C.emitReport(R);
}

void ento::registerUndefinedArraySubscriptChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefinedArraySubscriptChecker>();
}