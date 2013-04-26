
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
class TaintTesterChecker : public Checker< check::PostStmt<Expr> > {

  mutable OwningPtr<BugType> BT;
  void initBugType() const;

  /// Given a pointer argument, get the symbol of the value it contains
  /// (points to).
  SymbolRef getPointedToSymbol(CheckerContext &C,
                               const Expr* Arg,
                               bool IssueWarning = true) const;

public:
  void checkPostStmt(const Expr *E, CheckerContext &C) const;
};
}

inline void TaintTesterChecker::initBugType() const {
  if (!BT)
    BT.reset(new BugType("Tainted data", "General"));
}

void TaintTesterChecker::checkPostStmt(const Expr *E,
                                       CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  if (!State)
    return;

  if (State->isTainted(E, C.getLocationContext())) {
    if (ExplodedNode *N = C.addTransition()) {
      initBugType();
      BugReport *report = new BugReport(*BT, "tainted",N);
      report->addRange(E->getSourceRange());
      C.emitReport(report);
    }
  }
}

void ento::registerTaintTesterChecker(CheckerManager &mgr) {
  mgr.registerChecker<TaintTesterChecker>();
}