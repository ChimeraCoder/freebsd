
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

#include "Transforms.h"
#include "Internals.h"
#include "clang/AST/ASTContext.h"
#include "clang/Sema/SemaDiagnostic.h"

using namespace clang;
using namespace arcmt;
using namespace trans;

namespace {

class UnusedInitRewriter : public RecursiveASTVisitor<UnusedInitRewriter> {
  Stmt *Body;
  MigrationPass &Pass;

  ExprSet Removables;

public:
  UnusedInitRewriter(MigrationPass &pass)
    : Body(0), Pass(pass) { }

  void transformBody(Stmt *body, Decl *ParentD) {
    Body = body;
    collectRemovables(body, Removables);
    TraverseStmt(body);
  }

  bool VisitObjCMessageExpr(ObjCMessageExpr *ME) {
    if (ME->isDelegateInitCall() &&
        isRemovable(ME) &&
        Pass.TA.hasDiagnostic(diag::err_arc_unused_init_message,
                              ME->getExprLoc())) {
      Transaction Trans(Pass.TA);
      Pass.TA.clearDiagnostic(diag::err_arc_unused_init_message,
                              ME->getExprLoc());
      SourceRange ExprRange = ME->getSourceRange();
      Pass.TA.insert(ExprRange.getBegin(), "if (!(self = ");
      std::string retStr = ")) return ";
      retStr += getNilString(Pass.Ctx);
      Pass.TA.insertAfterToken(ExprRange.getEnd(), retStr);
    }
    return true;
  }

private:
  bool isRemovable(Expr *E) const {
    return Removables.count(E);
  }
};

} // anonymous namespace

void trans::rewriteUnusedInitDelegate(MigrationPass &pass) {
  BodyTransform<UnusedInitRewriter> trans(pass);
  trans.TraverseDecl(pass.Ctx.getTranslationUnitDecl());
}