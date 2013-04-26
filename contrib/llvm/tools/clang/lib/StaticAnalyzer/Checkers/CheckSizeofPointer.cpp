
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
#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"

using namespace clang;
using namespace ento;

namespace {
class WalkAST : public StmtVisitor<WalkAST> {
  BugReporter &BR;
  AnalysisDeclContext* AC;

public:
  WalkAST(BugReporter &br, AnalysisDeclContext* ac) : BR(br), AC(ac) {}
  void VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *E);
  void VisitStmt(Stmt *S) { VisitChildren(S); }
  void VisitChildren(Stmt *S);
};
}

void WalkAST::VisitChildren(Stmt *S) {
  for (Stmt::child_iterator I = S->child_begin(), E = S->child_end(); I!=E; ++I)
    if (Stmt *child = *I)
      Visit(child);
}

// CWE-467: Use of sizeof() on a Pointer Type
void WalkAST::VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *E) {
  if (E->getKind() != UETT_SizeOf)
    return;

  // If an explicit type is used in the code, usually the coder knows what he is
  // doing.
  if (E->isArgumentType())
    return;

  QualType T = E->getTypeOfArgument();
  if (T->isPointerType()) {

    // Many false positives have the form 'sizeof *p'. This is reasonable 
    // because people know what they are doing when they intentionally 
    // dereference the pointer.
    Expr *ArgEx = E->getArgumentExpr();
    if (!isa<DeclRefExpr>(ArgEx->IgnoreParens()))
      return;

    SourceRange R = ArgEx->getSourceRange();
    PathDiagnosticLocation ELoc =
      PathDiagnosticLocation::createBegin(E, BR.getSourceManager(), AC);
    BR.EmitBasicReport(AC->getDecl(),
                       "Potential unintended use of sizeof() on pointer type",
                       "Logic",
                       "The code calls sizeof() on a pointer type. "
                       "This can produce an unexpected result.",
                       ELoc, &R, 1);
  }
}

//===----------------------------------------------------------------------===//
// SizeofPointerChecker
//===----------------------------------------------------------------------===//

namespace {
class SizeofPointerChecker : public Checker<check::ASTCodeBody> {
public:
  void checkASTCodeBody(const Decl *D, AnalysisManager& mgr,
                        BugReporter &BR) const {
    WalkAST walker(BR, mgr.getAnalysisDeclContext(D));
    walker.Visit(D->getBody());
  }
};
}

void ento::registerSizeofPointerChecker(CheckerManager &mgr) {
  mgr.registerChecker<SizeofPointerChecker>();
}