
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

#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerHelpers.h"
#include "clang/AST/Expr.h"

// Recursively find any substatements containing macros
bool clang::ento::containsMacro(const Stmt *S) {
  if (S->getLocStart().isMacroID())
    return true;

  if (S->getLocEnd().isMacroID())
    return true;

  for (Stmt::const_child_iterator I = S->child_begin(); I != S->child_end();
      ++I)
    if (const Stmt *child = *I)
      if (containsMacro(child))
        return true;

  return false;
}

// Recursively find any substatements containing enum constants
bool clang::ento::containsEnum(const Stmt *S) {
  const DeclRefExpr *DR = dyn_cast<DeclRefExpr>(S);

  if (DR && isa<EnumConstantDecl>(DR->getDecl()))
    return true;

  for (Stmt::const_child_iterator I = S->child_begin(); I != S->child_end();
      ++I)
    if (const Stmt *child = *I)
      if (containsEnum(child))
        return true;

  return false;
}

// Recursively find any substatements containing static vars
bool clang::ento::containsStaticLocal(const Stmt *S) {
  const DeclRefExpr *DR = dyn_cast<DeclRefExpr>(S);

  if (DR)
    if (const VarDecl *VD = dyn_cast<VarDecl>(DR->getDecl()))
      if (VD->isStaticLocal())
        return true;

  for (Stmt::const_child_iterator I = S->child_begin(); I != S->child_end();
      ++I)
    if (const Stmt *child = *I)
      if (containsStaticLocal(child))
        return true;

  return false;
}

// Recursively find any substatements containing __builtin_offsetof
bool clang::ento::containsBuiltinOffsetOf(const Stmt *S) {
  if (isa<OffsetOfExpr>(S))
    return true;

  for (Stmt::const_child_iterator I = S->child_begin(); I != S->child_end();
      ++I)
    if (const Stmt *child = *I)
      if (containsBuiltinOffsetOf(child))
        return true;

  return false;
}