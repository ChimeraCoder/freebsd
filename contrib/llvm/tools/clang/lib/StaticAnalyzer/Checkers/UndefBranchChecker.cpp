
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

class UndefBranchChecker : public Checker<check::BranchCondition> {
  mutable OwningPtr<BuiltinBug> BT;

  struct FindUndefExpr {
    ProgramStateRef St;
    const LocationContext *LCtx;

    FindUndefExpr(ProgramStateRef S, const LocationContext *L) 
      : St(S), LCtx(L) {}

    const Expr *FindExpr(const Expr *Ex) {
      if (!MatchesCriteria(Ex))
        return 0;

      for (Stmt::const_child_iterator I = Ex->child_begin(), 
                                      E = Ex->child_end();I!=E;++I)
        if (const Expr *ExI = dyn_cast_or_null<Expr>(*I)) {
          const Expr *E2 = FindExpr(ExI);
          if (E2) return E2;
        }

      return Ex;
    }

    bool MatchesCriteria(const Expr *Ex) { 
      return St->getSVal(Ex, LCtx).isUndef();
    }
  };

public:
  void checkBranchCondition(const Stmt *Condition, CheckerContext &Ctx) const;
};

}

void UndefBranchChecker::checkBranchCondition(const Stmt *Condition,
                                              CheckerContext &Ctx) const {
  SVal X = Ctx.getState()->getSVal(Condition, Ctx.getLocationContext());
  if (X.isUndef()) {
    // Generate a sink node, which implicitly marks both outgoing branches as
    // infeasible.
    ExplodedNode *N = Ctx.generateSink();
    if (N) {
      if (!BT)
        BT.reset(
               new BuiltinBug("Branch condition evaluates to a garbage value"));

      // What's going on here: we want to highlight the subexpression of the
      // condition that is the most likely source of the "uninitialized
      // branch condition."  We do a recursive walk of the condition's
      // subexpressions and roughly look for the most nested subexpression
      // that binds to Undefined.  We then highlight that expression's range.

      // Get the predecessor node and check if is a PostStmt with the Stmt
      // being the terminator condition.  We want to inspect the state
      // of that node instead because it will contain main information about
      // the subexpressions.

      // Note: any predecessor will do.  They should have identical state,
      // since all the BlockEdge did was act as an error sink since the value
      // had to already be undefined.
      assert (!N->pred_empty());
      const Expr *Ex = cast<Expr>(Condition);
      ExplodedNode *PrevN = *N->pred_begin();
      ProgramPoint P = PrevN->getLocation();
      ProgramStateRef St = N->getState();

      if (Optional<PostStmt> PS = P.getAs<PostStmt>())
        if (PS->getStmt() == Ex)
          St = PrevN->getState();

      FindUndefExpr FindIt(St, Ctx.getLocationContext());
      Ex = FindIt.FindExpr(Ex);

      // Emit the bug report.
      BugReport *R = new BugReport(*BT, BT->getDescription(), N);
      bugreporter::trackNullOrUndefValue(N, Ex, *R);
      R->addRange(Ex->getSourceRange());

      Ctx.emitReport(R);
    }
  }
}

void ento::registerUndefBranchChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefBranchChecker>();
}