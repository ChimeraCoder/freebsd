
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

#include "clang/AST/ASTContext.h"
#include "clang/AST/ExprObjC.h"
#include "clang/Analysis/DomainSpecific/ObjCNoReturn.h"

using namespace clang;

static bool isSubclass(const ObjCInterfaceDecl *Class, IdentifierInfo *II) {
  if (!Class)
    return false;
  if (Class->getIdentifier() == II)
    return true;
  return isSubclass(Class->getSuperClass(), II);
}

ObjCNoReturn::ObjCNoReturn(ASTContext &C)
  : RaiseSel(GetNullarySelector("raise", C)),
    NSExceptionII(&C.Idents.get("NSException"))
{
  // Generate selectors.
  SmallVector<IdentifierInfo*, 3> II;
  
  // raise:format:
  II.push_back(&C.Idents.get("raise"));
  II.push_back(&C.Idents.get("format"));
  NSExceptionInstanceRaiseSelectors[0] =
    C.Selectors.getSelector(II.size(), &II[0]);
    
  // raise:format:arguments:
  II.push_back(&C.Idents.get("arguments"));
  NSExceptionInstanceRaiseSelectors[1] =
    C.Selectors.getSelector(II.size(), &II[0]);
}


bool ObjCNoReturn::isImplicitNoReturn(const ObjCMessageExpr *ME) {
  Selector S = ME->getSelector();
  
  if (ME->isInstanceMessage()) {
    // Check for the "raise" message.
    return S == RaiseSel;
  }

  if (const ObjCInterfaceDecl *ID = ME->getReceiverInterface()) {
    if (isSubclass(ID, NSExceptionII)) {
      for (unsigned i = 0; i < NUM_RAISE_SELECTORS; ++i) {
        if (S == NSExceptionInstanceRaiseSelectors[i])
          return true;
      }
    }
  }
  
  return false;
}