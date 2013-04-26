
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
#include "clang/AST/DeclObjC.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprObjC.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/PathDiagnostic.h"
#include "clang/StaticAnalyzer/Core/Checker.h"

using namespace clang;
using namespace ento;

enum IVarState { Unused, Used };
typedef llvm::DenseMap<const ObjCIvarDecl*,IVarState> IvarUsageMap;

static void Scan(IvarUsageMap& M, const Stmt *S) {
  if (!S)
    return;

  if (const ObjCIvarRefExpr *Ex = dyn_cast<ObjCIvarRefExpr>(S)) {
    const ObjCIvarDecl *D = Ex->getDecl();
    IvarUsageMap::iterator I = M.find(D);
    if (I != M.end())
      I->second = Used;
    return;
  }

  // Blocks can reference an instance variable of a class.
  if (const BlockExpr *BE = dyn_cast<BlockExpr>(S)) {
    Scan(M, BE->getBody());
    return;
  }

  if (const PseudoObjectExpr *POE = dyn_cast<PseudoObjectExpr>(S))
    for (PseudoObjectExpr::const_semantics_iterator
        i = POE->semantics_begin(), e = POE->semantics_end(); i != e; ++i) {
      const Expr *sub = *i;
      if (const OpaqueValueExpr *OVE = dyn_cast<OpaqueValueExpr>(sub))
        sub = OVE->getSourceExpr();
      Scan(M, sub);
    }

  for (Stmt::const_child_iterator I=S->child_begin(),E=S->child_end(); I!=E;++I)
    Scan(M, *I);
}

static void Scan(IvarUsageMap& M, const ObjCPropertyImplDecl *D) {
  if (!D)
    return;

  const ObjCIvarDecl *ID = D->getPropertyIvarDecl();

  if (!ID)
    return;

  IvarUsageMap::iterator I = M.find(ID);
  if (I != M.end())
    I->second = Used;
}

static void Scan(IvarUsageMap& M, const ObjCContainerDecl *D) {
  // Scan the methods for accesses.
  for (ObjCContainerDecl::instmeth_iterator I = D->instmeth_begin(),
       E = D->instmeth_end(); I!=E; ++I)
    Scan(M, (*I)->getBody());

  if (const ObjCImplementationDecl *ID = dyn_cast<ObjCImplementationDecl>(D)) {
    // Scan for @synthesized property methods that act as setters/getters
    // to an ivar.
    for (ObjCImplementationDecl::propimpl_iterator I = ID->propimpl_begin(),
         E = ID->propimpl_end(); I!=E; ++I)
      Scan(M, *I);

    // Scan the associated categories as well.
    for (ObjCInterfaceDecl::visible_categories_iterator
           Cat = ID->getClassInterface()->visible_categories_begin(),
           CatEnd = ID->getClassInterface()->visible_categories_end();
         Cat != CatEnd; ++Cat) {
      if (const ObjCCategoryImplDecl *CID = Cat->getImplementation())
        Scan(M, CID);
    }
  }
}

static void Scan(IvarUsageMap &M, const DeclContext *C, const FileID FID,
                 SourceManager &SM) {
  for (DeclContext::decl_iterator I=C->decls_begin(), E=C->decls_end();
       I!=E; ++I)
    if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(*I)) {
      SourceLocation L = FD->getLocStart();
      if (SM.getFileID(L) == FID)
        Scan(M, FD->getBody());
    }
}

static void checkObjCUnusedIvar(const ObjCImplementationDecl *D,
                                BugReporter &BR) {

  const ObjCInterfaceDecl *ID = D->getClassInterface();
  IvarUsageMap M;

  // Iterate over the ivars.
  for (ObjCInterfaceDecl::ivar_iterator I=ID->ivar_begin(),
        E=ID->ivar_end(); I!=E; ++I) {

    const ObjCIvarDecl *ID = *I;

    // Ignore ivars that...
    // (a) aren't private
    // (b) explicitly marked unused
    // (c) are iboutlets
    // (d) are unnamed bitfields
    if (ID->getAccessControl() != ObjCIvarDecl::Private ||
        ID->getAttr<UnusedAttr>() || ID->getAttr<IBOutletAttr>() ||
        ID->getAttr<IBOutletCollectionAttr>() ||
        ID->isUnnamedBitfield())
      continue;

    M[ID] = Unused;
  }

  if (M.empty())
    return;

  // Now scan the implementation declaration.
  Scan(M, D);

  // Any potentially unused ivars?
  bool hasUnused = false;
  for (IvarUsageMap::iterator I = M.begin(), E = M.end(); I!=E; ++I)
    if (I->second == Unused) {
      hasUnused = true;
      break;
    }

  if (!hasUnused)
    return;

  // We found some potentially unused ivars.  Scan the entire translation unit
  // for functions inside the @implementation that reference these ivars.
  // FIXME: In the future hopefully we can just use the lexical DeclContext
  // to go from the ObjCImplementationDecl to the lexically "nested"
  // C functions.
  SourceManager &SM = BR.getSourceManager();
  Scan(M, D->getDeclContext(), SM.getFileID(D->getLocation()), SM);

  // Find ivars that are unused.
  for (IvarUsageMap::iterator I = M.begin(), E = M.end(); I!=E; ++I)
    if (I->second == Unused) {
      std::string sbuf;
      llvm::raw_string_ostream os(sbuf);
      os << "Instance variable '" << *I->first << "' in class '" << *ID
         << "' is never used by the methods in its @implementation "
            "(although it may be used by category methods).";

      PathDiagnosticLocation L =
        PathDiagnosticLocation::create(I->first, BR.getSourceManager());
      BR.EmitBasicReport(D, "Unused instance variable", "Optimization",
                         os.str(), L);
    }
}

//===----------------------------------------------------------------------===//
// ObjCUnusedIvarsChecker
//===----------------------------------------------------------------------===//

namespace {
class ObjCUnusedIvarsChecker : public Checker<
                                      check::ASTDecl<ObjCImplementationDecl> > {
public:
  void checkASTDecl(const ObjCImplementationDecl *D, AnalysisManager& mgr,
                    BugReporter &BR) const {
    checkObjCUnusedIvar(D, BR);
  }
};
}

void ento::registerObjCUnusedIvarsChecker(CheckerManager &mgr) {
  mgr.registerChecker<ObjCUnusedIvarsChecker>();
}