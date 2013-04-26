
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

class APIChecker : public RecursiveASTVisitor<APIChecker> {
  MigrationPass &Pass;

  Selector getReturnValueSel, setReturnValueSel;
  Selector getArgumentSel, setArgumentSel;

  Selector zoneSel;
public:
  APIChecker(MigrationPass &pass) : Pass(pass) {
    SelectorTable &sels = Pass.Ctx.Selectors;
    IdentifierTable &ids = Pass.Ctx.Idents;
    getReturnValueSel = sels.getUnarySelector(&ids.get("getReturnValue"));
    setReturnValueSel = sels.getUnarySelector(&ids.get("setReturnValue"));

    IdentifierInfo *selIds[2];
    selIds[0] = &ids.get("getArgument");
    selIds[1] = &ids.get("atIndex");
    getArgumentSel = sels.getSelector(2, selIds);
    selIds[0] = &ids.get("setArgument");
    setArgumentSel = sels.getSelector(2, selIds);

    zoneSel = sels.getNullarySelector(&ids.get("zone"));
  }

  bool VisitObjCMessageExpr(ObjCMessageExpr *E) {
    // NSInvocation.
    if (E->isInstanceMessage() &&
        E->getReceiverInterface() &&
        E->getReceiverInterface()->getName() == "NSInvocation") {
      StringRef selName;
      if (E->getSelector() == getReturnValueSel)
        selName = "getReturnValue";
      else if (E->getSelector() == setReturnValueSel)
        selName = "setReturnValue";
      else if (E->getSelector() == getArgumentSel)
        selName = "getArgument";
      else if (E->getSelector() == setArgumentSel)
        selName = "setArgument";

      if (selName.empty())
        return true;

      Expr *parm = E->getArg(0)->IgnoreParenCasts();
      QualType pointee = parm->getType()->getPointeeType();
      if (pointee.isNull())
        return true;

      if (pointee.getObjCLifetime() > Qualifiers::OCL_ExplicitNone) {
        std::string err = "NSInvocation's ";
        err += selName;
        err += " is not safe to be used with an object with ownership other "
            "than __unsafe_unretained";
        Pass.TA.reportError(err, parm->getLocStart(), parm->getSourceRange());
      }
      return true;
    }

    // -zone.
    if (E->isInstanceMessage() &&
        E->getInstanceReceiver() &&
        E->getSelector() == zoneSel &&
        Pass.TA.hasDiagnostic(diag::err_unavailable,
                              diag::err_unavailable_message,
                              E->getInstanceReceiver()->getExprLoc())) {
      // Calling -zone is meaningless in ARC, change it to nil.
      Transaction Trans(Pass.TA);
      Pass.TA.clearDiagnostic(diag::err_unavailable,
                              diag::err_unavailable_message,
                              E->getInstanceReceiver()->getExprLoc());
      Pass.TA.replace(E->getSourceRange(), getNilString(Pass.Ctx));
    }
    return true;
  }
};

} // anonymous namespace

void trans::checkAPIUses(MigrationPass &pass) {
  APIChecker(pass).TraverseDecl(pass.Ctx.getTranslationUnitDecl());
}