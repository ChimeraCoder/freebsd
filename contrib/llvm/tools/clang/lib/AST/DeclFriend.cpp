
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
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclTemplate.h"
using namespace clang;

void FriendDecl::anchor() { }

FriendDecl *FriendDecl::getNextFriendSlowCase() {
  return cast_or_null<FriendDecl>(
                           NextFriend.get(getASTContext().getExternalSource()));
}

FriendDecl *FriendDecl::Create(ASTContext &C, DeclContext *DC,
                               SourceLocation L,
                               FriendUnion Friend,
                               SourceLocation FriendL,
                        ArrayRef<TemplateParameterList*> FriendTypeTPLists) {
#ifndef NDEBUG
  if (Friend.is<NamedDecl*>()) {
    NamedDecl *D = Friend.get<NamedDecl*>();
    assert(isa<FunctionDecl>(D) ||
           isa<CXXRecordDecl>(D) ||
           isa<FunctionTemplateDecl>(D) ||
           isa<ClassTemplateDecl>(D));

    // As a temporary hack, we permit template instantiation to point
    // to the original declaration when instantiating members.
    assert(D->getFriendObjectKind() ||
           (cast<CXXRecordDecl>(DC)->getTemplateSpecializationKind()));
    // These template parameters are for friend types only.
    assert(FriendTypeTPLists.size() == 0);
  }
#endif

  std::size_t Size = sizeof(FriendDecl)
    + FriendTypeTPLists.size() * sizeof(TemplateParameterList*);
  void *Mem = C.Allocate(Size);
  FriendDecl *FD = new (Mem) FriendDecl(DC, L, Friend, FriendL,
                                        FriendTypeTPLists);
  cast<CXXRecordDecl>(DC)->pushFriendDecl(FD);
  return FD;
}

FriendDecl *FriendDecl::CreateDeserialized(ASTContext &C, unsigned ID,
                                           unsigned FriendTypeNumTPLists) {
  std::size_t Size = sizeof(FriendDecl)
    + FriendTypeNumTPLists * sizeof(TemplateParameterList*);
  void *Mem = AllocateDeserializedDecl(C, ID, Size);
  return new (Mem) FriendDecl(EmptyShell(), FriendTypeNumTPLists);
}