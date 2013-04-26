
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

#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

void MCOperand::print(raw_ostream &OS, const MCAsmInfo *MAI) const {
  OS << "<MCOperand ";
  if (!isValid())
    OS << "INVALID";
  else if (isReg())
    OS << "Reg:" << getReg();
  else if (isImm())
    OS << "Imm:" << getImm();
  else if (isExpr()) {
    OS << "Expr:(" << *getExpr() << ")";
  } else if (isInst()) {
    OS << "Inst:(" << *getInst() << ")";
  } else
    OS << "UNDEFINED";
  OS << ">";
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void MCOperand::dump() const {
  print(dbgs(), 0);
  dbgs() << "\n";
}
#endif

void MCInst::print(raw_ostream &OS, const MCAsmInfo *MAI) const {
  OS << "<MCInst " << getOpcode();
  for (unsigned i = 0, e = getNumOperands(); i != e; ++i) {
    OS << " ";
    getOperand(i).print(OS, MAI);
  }
  OS << ">";
}

void MCInst::dump_pretty(raw_ostream &OS, const MCAsmInfo *MAI,
                         const MCInstPrinter *Printer,
                         StringRef Separator) const {
  OS << "<MCInst #" << getOpcode();

  // Show the instruction opcode name if we have access to a printer.
  if (Printer)
    OS << ' ' << Printer->getOpcodeName(getOpcode());

  for (unsigned i = 0, e = getNumOperands(); i != e; ++i) {
    OS << Separator;
    getOperand(i).print(OS, MAI);
  }
  OS << ">";
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void MCInst::dump() const {
  print(dbgs(), 0);
  dbgs() << "\n";
}
#endif