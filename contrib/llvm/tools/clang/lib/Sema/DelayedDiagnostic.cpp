
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
#include "clang/Sema/DelayedDiagnostic.h"
#include <string.h>
using namespace clang;
using namespace sema;

DelayedDiagnostic DelayedDiagnostic::makeDeprecation(SourceLocation Loc,
                                    const NamedDecl *D,
                                    const ObjCInterfaceDecl *UnknownObjCClass,
                                    const ObjCPropertyDecl  *ObjCProperty,
                                    StringRef Msg) {
  DelayedDiagnostic DD;
  DD.Kind = Deprecation;
  DD.Triggered = false;
  DD.Loc = Loc;
  DD.DeprecationData.Decl = D;
  DD.DeprecationData.UnknownObjCClass = UnknownObjCClass;
  DD.DeprecationData.ObjCProperty = ObjCProperty;
  char *MessageData = 0;
  if (Msg.size()) {
    MessageData = new char [Msg.size()];
    memcpy(MessageData, Msg.data(), Msg.size());
  }

  DD.DeprecationData.Message = MessageData;
  DD.DeprecationData.MessageLen = Msg.size();
  return DD;
}

void DelayedDiagnostic::Destroy() {
  switch (Kind) {
  case Access: 
    getAccessData().~AccessedEntity(); 
    break;

  case Deprecation: 
    delete [] DeprecationData.Message;
    break;

  case ForbiddenType:
    break;
  }
}