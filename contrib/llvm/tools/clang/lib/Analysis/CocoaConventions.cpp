
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

#include "clang/Analysis/DomainSpecific/CocoaConventions.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/Type.h"
#include "clang/Basic/CharInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/ErrorHandling.h"

using namespace clang;
using namespace ento;

bool cocoa::isRefType(QualType RetTy, StringRef Prefix,
                      StringRef Name) {
  // Recursively walk the typedef stack, allowing typedefs of reference types.
  while (const TypedefType *TD = dyn_cast<TypedefType>(RetTy.getTypePtr())) {
    StringRef TDName = TD->getDecl()->getIdentifier()->getName();
    if (TDName.startswith(Prefix) && TDName.endswith("Ref"))
      return true;
    // XPC unfortunately uses CF-style function names, but aren't CF types.
    if (TDName.startswith("xpc_"))
      return false;
    RetTy = TD->getDecl()->getUnderlyingType();
  }
  
  if (Name.empty())
    return false;
  
  // Is the type void*?
  const PointerType* PT = RetTy->getAs<PointerType>();
  if (!(PT->getPointeeType().getUnqualifiedType()->isVoidType()))
    return false;
  
  // Does the name start with the prefix?
  return Name.startswith(Prefix);
}

bool coreFoundation::isCFObjectRef(QualType T) {
  return cocoa::isRefType(T, "CF") || // Core Foundation.
         cocoa::isRefType(T, "CG") || // Core Graphics.
         cocoa::isRefType(T, "DADisk") || // Disk Arbitration API.
         cocoa::isRefType(T, "DADissenter") ||
         cocoa::isRefType(T, "DASessionRef");
}


bool cocoa::isCocoaObjectRef(QualType Ty) {
  if (!Ty->isObjCObjectPointerType())
    return false;
  
  const ObjCObjectPointerType *PT = Ty->getAs<ObjCObjectPointerType>();
  
  // Can be true for objects with the 'NSObject' attribute.
  if (!PT)
    return true;
  
  // We assume that id<..>, id, Class, and Class<..> all represent tracked
  // objects.
  if (PT->isObjCIdType() || PT->isObjCQualifiedIdType() ||
      PT->isObjCClassType() || PT->isObjCQualifiedClassType())
    return true;
  
  // Does the interface subclass NSObject?
  // FIXME: We can memoize here if this gets too expensive.
  const ObjCInterfaceDecl *ID = PT->getInterfaceDecl();
  
  // Assume that anything declared with a forward declaration and no
  // @interface subclasses NSObject.
  if (!ID->hasDefinition())
    return true;
  
  for ( ; ID ; ID = ID->getSuperClass())
    if (ID->getIdentifier()->getName() == "NSObject")
      return true;
  
  return false;
}

bool coreFoundation::followsCreateRule(const FunctionDecl *fn) {
  // For now, *just* base this on the function name, not on anything else.

  const IdentifierInfo *ident = fn->getIdentifier();
  if (!ident) return false;
  StringRef functionName = ident->getName();
  
  StringRef::iterator it = functionName.begin();
  StringRef::iterator start = it;
  StringRef::iterator endI = functionName.end();
    
  while (true) {
    // Scan for the start of 'create' or 'copy'.
    for ( ; it != endI ; ++it) {
      // Search for the first character.  It can either be 'C' or 'c'.
      char ch = *it;
      if (ch == 'C' || ch == 'c') {
        // Make sure this isn't something like 'recreate' or 'Scopy'.
        if (ch == 'c' && it != start && isLetter(*(it - 1)))
          continue;

        ++it;
        break;
      }
    }

    // Did we hit the end of the string?  If so, we didn't find a match.
    if (it == endI)
      return false;
    
    // Scan for *lowercase* 'reate' or 'opy', followed by no lowercase
    // character.
    StringRef suffix = functionName.substr(it - start);
    if (suffix.startswith("reate")) {
      it += 5;
    }
    else if (suffix.startswith("opy")) {
      it += 3;
    } else {
      // Keep scanning.
      continue;
    }
    
    if (it == endI || !isLowercase(*it))
      return true;
  
    // If we matched a lowercase character, it isn't the end of the
    // word.  Keep scanning.
  }
}