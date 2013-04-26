
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
#include "clang/AST/Attr.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ento;

namespace {
class UndefCapturedBlockVarChecker
  : public Checker< check::PostStmt<BlockExpr> > {
 mutable OwningPtr<BugType> BT;

public:
  void checkPostStmt(const BlockExpr *BE, CheckerContext &C) const;
};
} // end anonymous namespace

static const DeclRefExpr *FindBlockDeclRefExpr(const Stmt *S,
                                               const VarDecl *VD) {
  if (const DeclRefExpr *BR = dyn_cast<DeclRefExpr>(S))
    if (BR->getDecl() == VD)
      return BR;

  for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
       I!=E; ++I)
    if (const Stmt *child = *I) {
      const DeclRefExpr *BR = FindBlockDeclRefExpr(child, VD);
      if (BR)
        return BR;
    }

  return NULL;
}

void
UndefCapturedBlockVarChecker::checkPostStmt(const BlockExpr *BE,
                                            CheckerContext &C) const {
  if (!BE->getBlockDecl()->hasCaptures())
    return;

  ProgramStateRef state = C.getState();
  const BlockDataRegion *R =
    cast<BlockDataRegion>(state->getSVal(BE,
                                         C.getLocationContext()).getAsRegion());

  BlockDataRegion::referenced_vars_iterator I = R->referenced_vars_begin(),
                                            E = R->referenced_vars_end();

  for (; I != E; ++I) {
    // This VarRegion is the region associated with the block; we need
    // the one associated with the encompassing context.
    const VarRegion *VR = I.getCapturedRegion();
    const VarDecl *VD = VR->getDecl();

    if (VD->getAttr<BlocksAttr>() || !VD->hasLocalStorage())
      continue;

    // Get the VarRegion associated with VD in the local stack frame.
    if (Optional<UndefinedVal> V =
          state->getSVal(I.getOriginalRegion()).getAs<UndefinedVal>()) {
      if (ExplodedNode *N = C.generateSink()) {
        if (!BT)
          BT.reset(new BuiltinBug("uninitialized variable captured by block"));

        // Generate a bug report.
        SmallString<128> buf;
        llvm::raw_svector_ostream os(buf);

        os << "Variable '" << VD->getName() 
           << "' is uninitialized when captured by block";

        BugReport *R = new BugReport(*BT, os.str(), N);
        if (const Expr *Ex = FindBlockDeclRefExpr(BE->getBody(), VD))
          R->addRange(Ex->getSourceRange());
        R->addVisitor(new FindLastStoreBRVisitor(*V, VR,
                                             /*EnableNullFPSuppression*/false));
        R->disablePathPruning();
        // need location of block
        C.emitReport(R);
      }
    }
  }
}

void ento::registerUndefCapturedBlockVarChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefCapturedBlockVarChecker>();
}