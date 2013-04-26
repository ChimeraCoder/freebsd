
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

#include "clang/Basic/OpenMPKinds.h"
#include "clang/Basic/IdentifierTable.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>

using namespace clang;

OpenMPDirectiveKind clang::getOpenMPDirectiveKind(StringRef Str) {
  return llvm::StringSwitch<OpenMPDirectiveKind>(Str)
#define OPENMP_DIRECTIVE(Name) \
           .Case(#Name, OMPD_##Name)
#include "clang/Basic/OpenMPKinds.def"
           .Default(OMPD_unknown);
}

const char *clang::getOpenMPDirectiveName(OpenMPDirectiveKind Kind) {
  assert(Kind < NUM_OPENMP_DIRECTIVES);
  switch (Kind) {
  case OMPD_unknown:
    return ("unknown");
#define OPENMP_DIRECTIVE(Name) \
  case OMPD_##Name : return #Name;
#include "clang/Basic/OpenMPKinds.def"
  default:
    break;
  }
  llvm_unreachable("Invalid OpenMP directive kind");
}