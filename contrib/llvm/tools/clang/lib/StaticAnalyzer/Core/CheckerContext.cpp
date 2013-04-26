
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

#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/Basic/Builtins.h"
#include "clang/Lex/Lexer.h"

using namespace clang;
using namespace ento;

const FunctionDecl *CheckerContext::getCalleeDecl(const CallExpr *CE) const {
  ProgramStateRef State = getState();
  const Expr *Callee = CE->getCallee();
  SVal L = State->getSVal(Callee, Pred->getLocationContext());
  return L.getAsFunctionDecl();
}

StringRef CheckerContext::getCalleeName(const FunctionDecl *FunDecl) const {
  if (!FunDecl)
    return StringRef();
  IdentifierInfo *funI = FunDecl->getIdentifier();
  if (!funI)
    return StringRef();
  return funI->getName();
}


bool CheckerContext::isCLibraryFunction(const FunctionDecl *FD,
                                        StringRef Name) {
  // To avoid false positives (Ex: finding user defined functions with
  // similar names), only perform fuzzy name matching when it's a builtin.
  // Using a string compare is slow, we might want to switch on BuiltinID here.
  unsigned BId = FD->getBuiltinID();
  if (BId != 0) {
    if (Name.empty())
      return true;
    StringRef BName = FD->getASTContext().BuiltinInfo.GetName(BId);
    if (BName.find(Name) != StringRef::npos)
      return true;
  }

  const IdentifierInfo *II = FD->getIdentifier();
  // If this is a special C++ name without IdentifierInfo, it can't be a
  // C library function.
  if (!II)
    return false;

  // Look through 'extern "C"' and anything similar invented in the future.
  const DeclContext *DC = FD->getDeclContext();
  while (DC->isTransparentContext())
    DC = DC->getParent();

  // If this function is in a namespace, it is not a C library function.
  if (!DC->isTranslationUnit())
    return false;

  // If this function is not externally visible, it is not a C library function.
  // Note that we make an exception for inline functions, which may be
  // declared in header files without external linkage.
  if (!FD->isInlined() && FD->getLinkage() != ExternalLinkage)
    return false;

  if (Name.empty())
    return true;

  StringRef FName = II->getName();
  if (FName.equals(Name))
    return true;

  if (FName.startswith("__inline") && (FName.find(Name) != StringRef::npos))
    return true;

  if (FName.startswith("__") && FName.endswith("_chk") &&
      FName.find(Name) != StringRef::npos)
    return true;

  return false;
}

StringRef CheckerContext::getMacroNameOrSpelling(SourceLocation &Loc) {
  if (Loc.isMacroID())
    return Lexer::getImmediateMacroName(Loc, getSourceManager(),
                                             getLangOpts());
  SmallVector<char, 16> buf;
  return Lexer::getSpelling(Loc, buf, getSourceManager(), getLangOpts());
}