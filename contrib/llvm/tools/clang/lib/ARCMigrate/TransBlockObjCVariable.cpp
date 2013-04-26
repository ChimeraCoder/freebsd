
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
#include "clang/AST/Attr.h"
#include "clang/Basic/SourceManager.h"

using namespace clang;
using namespace arcmt;
using namespace trans;

namespace {

class RootBlockObjCVarRewriter :
                          public RecursiveASTVisitor<RootBlockObjCVarRewriter> {
  llvm::DenseSet<VarDecl *> &VarsToChange;

  class BlockVarChecker : public RecursiveASTVisitor<BlockVarChecker> {
    VarDecl *Var;
  
    typedef RecursiveASTVisitor<BlockVarChecker> base;
  public:
    BlockVarChecker(VarDecl *var) : Var(var) { }
  
    bool TraverseImplicitCastExpr(ImplicitCastExpr *castE) {
      if (DeclRefExpr *
            ref = dyn_cast<DeclRefExpr>(castE->getSubExpr())) {
        if (ref->getDecl() == Var) {
          if (castE->getCastKind() == CK_LValueToRValue)
            return true; // Using the value of the variable.
          if (castE->getCastKind() == CK_NoOp && castE->isLValue() &&
              Var->getASTContext().getLangOpts().CPlusPlus)
            return true; // Binding to const C++ reference.
        }
      }

      return base::TraverseImplicitCastExpr(castE);
    }

    bool VisitDeclRefExpr(DeclRefExpr *E) {
      if (E->getDecl() == Var)
        return false; // The reference of the variable, and not just its value,
                      //  is needed.
      return true;
    }
  };

public:
  RootBlockObjCVarRewriter(llvm::DenseSet<VarDecl *> &VarsToChange)
    : VarsToChange(VarsToChange) { }

  bool VisitBlockDecl(BlockDecl *block) {
    SmallVector<VarDecl *, 4> BlockVars;
    
    for (BlockDecl::capture_iterator
           I = block->capture_begin(), E = block->capture_end(); I != E; ++I) {
      VarDecl *var = I->getVariable();
      if (I->isByRef() &&
          var->getType()->isObjCObjectPointerType() &&
          isImplicitStrong(var->getType())) {
        BlockVars.push_back(var);
      }
    }

    for (unsigned i = 0, e = BlockVars.size(); i != e; ++i) {
      VarDecl *var = BlockVars[i];

      BlockVarChecker checker(var);
      bool onlyValueOfVarIsNeeded = checker.TraverseStmt(block->getBody());
      if (onlyValueOfVarIsNeeded)
        VarsToChange.insert(var);
      else
        VarsToChange.erase(var);
    }

    return true;
  }

private:
  bool isImplicitStrong(QualType ty) {
    if (isa<AttributedType>(ty.getTypePtr()))
      return false;
    return ty.getLocalQualifiers().getObjCLifetime() == Qualifiers::OCL_Strong;
  }
};

class BlockObjCVarRewriter : public RecursiveASTVisitor<BlockObjCVarRewriter> {
  llvm::DenseSet<VarDecl *> &VarsToChange;

public:
  BlockObjCVarRewriter(llvm::DenseSet<VarDecl *> &VarsToChange)
    : VarsToChange(VarsToChange) { }

  bool TraverseBlockDecl(BlockDecl *block) {
    RootBlockObjCVarRewriter(VarsToChange).TraverseDecl(block);
    return true;
  }
};

} // anonymous namespace

void BlockObjCVariableTraverser::traverseBody(BodyContext &BodyCtx) {
  MigrationPass &Pass = BodyCtx.getMigrationContext().Pass;
  llvm::DenseSet<VarDecl *> VarsToChange;

  BlockObjCVarRewriter trans(VarsToChange);
  trans.TraverseStmt(BodyCtx.getTopStmt());

  for (llvm::DenseSet<VarDecl *>::iterator
         I = VarsToChange.begin(), E = VarsToChange.end(); I != E; ++I) {
    VarDecl *var = *I;
    BlocksAttr *attr = var->getAttr<BlocksAttr>();
    if(!attr)
      continue;
    bool useWeak = canApplyWeak(Pass.Ctx, var->getType());
    SourceManager &SM = Pass.Ctx.getSourceManager();
    Transaction Trans(Pass.TA);
    Pass.TA.replaceText(SM.getExpansionLoc(attr->getLocation()),
                        "__block",
                        useWeak ? "__weak" : "__unsafe_unretained");
  }
}