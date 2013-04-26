
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
class CastToStructChecker : public Checker< check::PreStmt<CastExpr> > {
  mutable OwningPtr<BuiltinBug> BT;

public:
  void checkPreStmt(const CastExpr *CE, CheckerContext &C) const;
};
}

void CastToStructChecker::checkPreStmt(const CastExpr *CE,
                                       CheckerContext &C) const {
  const Expr *E = CE->getSubExpr();
  ASTContext &Ctx = C.getASTContext();
  QualType OrigTy = Ctx.getCanonicalType(E->getType());
  QualType ToTy = Ctx.getCanonicalType(CE->getType());

  const PointerType *OrigPTy = dyn_cast<PointerType>(OrigTy.getTypePtr());
  const PointerType *ToPTy = dyn_cast<PointerType>(ToTy.getTypePtr());

  if (!ToPTy || !OrigPTy)
    return;

  QualType OrigPointeeTy = OrigPTy->getPointeeType();
  QualType ToPointeeTy = ToPTy->getPointeeType();

  if (!ToPointeeTy->isStructureOrClassType())
    return;

  // We allow cast from void*.
  if (OrigPointeeTy->isVoidType())
    return;

  // Now the cast-to-type is struct pointer, the original type is not void*.
  if (!OrigPointeeTy->isRecordType()) {
    if (ExplodedNode *N = C.addTransition()) {
      if (!BT)
        BT.reset(new BuiltinBug("Cast from non-struct type to struct type",
                            "Casting a non-structure type to a structure type "
                            "and accessing a field can lead to memory access "
                            "errors or data corruption."));
      BugReport *R = new BugReport(*BT,BT->getDescription(), N);
      R->addRange(CE->getSourceRange());
      C.emitReport(R);
    }
  }
}

void ento::registerCastToStructChecker(CheckerManager &mgr) {
  mgr.registerChecker<CastToStructChecker>();
}