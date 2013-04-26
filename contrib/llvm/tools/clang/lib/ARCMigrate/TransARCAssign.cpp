
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

class ARCAssignChecker : public RecursiveASTVisitor<ARCAssignChecker> {
  MigrationPass &Pass;
  llvm::DenseSet<VarDecl *> ModifiedVars;

public:
  ARCAssignChecker(MigrationPass &pass) : Pass(pass) { }

  bool VisitBinaryOperator(BinaryOperator *Exp) {
    if (Exp->getType()->isDependentType())
      return true;

    Expr *E = Exp->getLHS();
    SourceLocation OrigLoc = E->getExprLoc();
    SourceLocation Loc = OrigLoc;
    DeclRefExpr *declRef = dyn_cast<DeclRefExpr>(E->IgnoreParenCasts());
    if (declRef && isa<VarDecl>(declRef->getDecl())) {
      ASTContext &Ctx = Pass.Ctx;
      Expr::isModifiableLvalueResult IsLV = E->isModifiableLvalue(Ctx, &Loc);
      if (IsLV != Expr::MLV_ConstQualified)
        return true;
      VarDecl *var = cast<VarDecl>(declRef->getDecl());
      if (var->isARCPseudoStrong()) {
        Transaction Trans(Pass.TA);
        if (Pass.TA.clearDiagnostic(diag::err_typecheck_arr_assign_enumeration,
                                    Exp->getOperatorLoc())) {
          if (!ModifiedVars.count(var)) {
            TypeLoc TLoc = var->getTypeSourceInfo()->getTypeLoc();
            Pass.TA.insert(TLoc.getBeginLoc(), "__strong ");
            ModifiedVars.insert(var);
          }
        }
      }
    }
    
    return true;
  }
};

} // anonymous namespace

void trans::makeAssignARCSafe(MigrationPass &pass) {
  ARCAssignChecker assignCheck(pass);
  assignCheck.TraverseDecl(pass.Ctx.getTranslationUnitDecl());
}