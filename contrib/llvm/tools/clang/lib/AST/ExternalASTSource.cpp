
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

#include "clang/AST/ExternalASTSource.h"
#include "clang/AST/DeclarationName.h"

using namespace clang;

ExternalASTSource::~ExternalASTSource() { }

void ExternalASTSource::PrintStats() { }

Decl *ExternalASTSource::GetExternalDecl(uint32_t ID) {
  return 0;
}

Selector ExternalASTSource::GetExternalSelector(uint32_t ID) {
  return Selector();
}

uint32_t ExternalASTSource::GetNumExternalSelectors() {
   return 0;
}

Stmt *ExternalASTSource::GetExternalDeclStmt(uint64_t Offset) {
  return 0;
}

CXXBaseSpecifier *
ExternalASTSource::GetExternalCXXBaseSpecifiers(uint64_t Offset) {
  return 0;
}

bool
ExternalASTSource::FindExternalVisibleDeclsByName(const DeclContext *DC,
                                                  DeclarationName Name) {
  return false;
}

void ExternalASTSource::completeVisibleDeclsMap(const DeclContext *DC) {
}

ExternalLoadResult
ExternalASTSource::FindExternalLexicalDecls(const DeclContext *DC,
                                            bool (*isKindWeWant)(Decl::Kind),
                                         SmallVectorImpl<Decl*> &Result) {
  return ELR_AlreadyLoaded;
}

void ExternalASTSource::getMemoryBufferSizes(MemoryBufferSizes &sizes) const { }