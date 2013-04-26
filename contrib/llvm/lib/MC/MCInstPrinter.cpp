
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

#include "llvm/MC/MCInstPrinter.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

MCInstPrinter::~MCInstPrinter() {
}

/// getOpcodeName - Return the name of the specified opcode enum (e.g.
/// "MOV32ri") or empty if we can't resolve it.
StringRef MCInstPrinter::getOpcodeName(unsigned Opcode) const {
  return MII.getName(Opcode);
}

void MCInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
  llvm_unreachable("Target should implement this");
}

void MCInstPrinter::printAnnotation(raw_ostream &OS, StringRef Annot) {
  if (!Annot.empty()) {
    if (CommentStream)
      (*CommentStream) << Annot;
    else
      OS << " " << MAI.getCommentString() << " " << Annot;
  }
}

/// Utility functions to make adding mark ups simpler.
StringRef MCInstPrinter::markup(StringRef s) const {
  if (getUseMarkup())
    return s;
  else
    return "";
}
StringRef MCInstPrinter::markup(StringRef a, StringRef b) const {
  if (getUseMarkup())
    return a;
  else
    return b;
}

/// Utility function to print immediates in decimal or hex.
format_object1<int64_t> MCInstPrinter::formatImm(const int64_t Value) const {
  if (getPrintImmHex())
    return format("0x%" PRIx64, Value);
  else
    return format("%" PRId64, Value);
}