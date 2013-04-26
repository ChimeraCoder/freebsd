
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

#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

// Sentinel value for the absolute pseudo section.
const MCSection *MCSymbol::AbsolutePseudoSection =
  reinterpret_cast<const MCSection *>(1);

static bool isAcceptableChar(char C) {
  if ((C < 'a' || C > 'z') &&
      (C < 'A' || C > 'Z') &&
      (C < '0' || C > '9') &&
      C != '_' && C != '$' && C != '.' && C != '@')
    return false;
  return true;
}

/// NameNeedsQuoting - Return true if the identifier \p Str needs quotes to be
/// syntactically correct.
static bool NameNeedsQuoting(StringRef Str) {
  assert(!Str.empty() && "Cannot create an empty MCSymbol");

  // If any of the characters in the string is an unacceptable character, force
  // quotes.
  for (unsigned i = 0, e = Str.size(); i != e; ++i)
    if (!isAcceptableChar(Str[i]))
      return true;
  return false;
}

const MCSymbol &MCSymbol::AliasedSymbol() const {
  const MCSymbol *S = this;
  while (S->isVariable()) {
    const MCExpr *Value = S->getVariableValue();
    if (Value->getKind() != MCExpr::SymbolRef)
      return *S;
    const MCSymbolRefExpr *Ref = static_cast<const MCSymbolRefExpr*>(Value);
    S = &Ref->getSymbol();
  }
  return *S;
}

void MCSymbol::setVariableValue(const MCExpr *Value) {
  assert(!IsUsed && "Cannot set a variable that has already been used.");
  assert(Value && "Invalid variable value!");
  this->Value = Value;

  // Variables should always be marked as in the same "section" as the value.
  const MCSection *Section = Value->FindAssociatedSection();
  if (Section)
    setSection(*Section);
  else
    setUndefined();
}

void MCSymbol::print(raw_ostream &OS) const {
  // The name for this MCSymbol is required to be a valid target name.  However,
  // some targets support quoting names with funny characters.  If the name
  // contains a funny character, then print it quoted.
  if (!NameNeedsQuoting(getName())) {
    OS << getName();
    return;
  }

  OS << '"' << getName() << '"';
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void MCSymbol::dump() const {
  print(dbgs());
}
#endif