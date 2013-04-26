
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

class GCCollectableCallsChecker :
                         public RecursiveASTVisitor<GCCollectableCallsChecker> {
  MigrationContext &MigrateCtx;
  IdentifierInfo *NSMakeCollectableII;
  IdentifierInfo *CFMakeCollectableII;

public:
  GCCollectableCallsChecker(MigrationContext &ctx)
    : MigrateCtx(ctx) {
    IdentifierTable &Ids = MigrateCtx.Pass.Ctx.Idents;
    NSMakeCollectableII = &Ids.get("NSMakeCollectable");
    CFMakeCollectableII = &Ids.get("CFMakeCollectable");
  }

  bool shouldWalkTypesOfTypeLocs() const { return false; }

  bool VisitCallExpr(CallExpr *E) {
    TransformActions &TA = MigrateCtx.Pass.TA;

    if (MigrateCtx.isGCOwnedNonObjC(E->getType())) {
      if (MigrateCtx.Pass.noNSAllocReallocError())
        TA.reportWarning("call returns pointer to GC managed memory; "
                       "it will become unmanaged in ARC",
                       E->getLocStart(), E->getSourceRange());
      else 
        TA.reportError("call returns pointer to GC managed memory; "
                       "it will become unmanaged in ARC",
                       E->getLocStart(), E->getSourceRange());
      return true;
    }

    Expr *CEE = E->getCallee()->IgnoreParenImpCasts();
    if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(CEE)) {
      if (FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(DRE->getDecl())) {
        if (!FD->getDeclContext()->getRedeclContext()->isFileContext())
          return true;

        if (FD->getIdentifier() == NSMakeCollectableII) {
          Transaction Trans(TA);
          TA.clearDiagnostic(diag::err_unavailable,
                             diag::err_unavailable_message,
                             diag::err_ovl_deleted_call, // ObjC++
                             DRE->getSourceRange());
          TA.replace(DRE->getSourceRange(), "CFBridgingRelease");

        } else if (FD->getIdentifier() == CFMakeCollectableII) {
          TA.reportError("CFMakeCollectable will leak the object that it "
                         "receives in ARC", DRE->getLocation(),
                         DRE->getSourceRange());
        }
      }
    }

    return true;
  }
};

} // anonymous namespace

void GCCollectableCallsTraverser::traverseBody(BodyContext &BodyCtx) {
  GCCollectableCallsChecker(BodyCtx.getMigrationContext())
                                            .TraverseStmt(BodyCtx.getTopStmt());
}