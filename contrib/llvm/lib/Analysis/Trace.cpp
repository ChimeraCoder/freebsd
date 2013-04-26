
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

#include "llvm/Analysis/Trace.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

Function *Trace::getFunction() const {
  return getEntryBasicBlock()->getParent();
}

Module *Trace::getModule() const {
  return getFunction()->getParent();
}

/// print - Write trace to output stream.
///
void Trace::print(raw_ostream &O) const {
  Function *F = getFunction();
  O << "; Trace from function " << F->getName() << ", blocks:\n";
  for (const_iterator i = begin(), e = end(); i != e; ++i) {
    O << "; ";
    WriteAsOperand(O, *i, true, getModule());
    O << "\n";
  }
  O << "; Trace parent function: \n" << *F;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// dump - Debugger convenience method; writes trace to standard error
/// output stream.
///
void Trace::dump() const {
  print(dbgs());
}
#endif