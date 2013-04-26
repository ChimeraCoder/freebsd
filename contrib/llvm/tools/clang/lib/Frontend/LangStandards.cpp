
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

#include "clang/Frontend/LangStandard.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"
using namespace clang;
using namespace clang::frontend;

#define LANGSTANDARD(id, name, desc, features) \
  static const LangStandard Lang_##id = { name, desc, features };
#include "clang/Frontend/LangStandards.def"

const LangStandard &LangStandard::getLangStandardForKind(Kind K) {
  switch (K) {
  case lang_unspecified:
    llvm::report_fatal_error("getLangStandardForKind() on unspecified kind");
#define LANGSTANDARD(id, name, desc, features) \
    case lang_##id: return Lang_##id;
#include "clang/Frontend/LangStandards.def"
  }
  llvm_unreachable("Invalid language kind!");
}

const LangStandard *LangStandard::getLangStandardForName(StringRef Name) {
  Kind K = llvm::StringSwitch<Kind>(Name)
#define LANGSTANDARD(id, name, desc, features) \
    .Case(name, lang_##id)
#include "clang/Frontend/LangStandards.def"
    .Default(lang_unspecified);
  if (K == lang_unspecified)
    return 0;

  return &getLangStandardForKind(K);
}