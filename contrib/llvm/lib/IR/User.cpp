
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

#include "llvm/IR/User.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Operator.h"

namespace llvm {

//===----------------------------------------------------------------------===//
//                                 User Class
//===----------------------------------------------------------------------===//

void User::anchor() {}

// replaceUsesOfWith - Replaces all references to the "From" definition with
// references to the "To" definition.
//
void User::replaceUsesOfWith(Value *From, Value *To) {
  if (From == To) return;   // Duh what?

  assert((!isa<Constant>(this) || isa<GlobalValue>(this)) &&
         "Cannot call User::replaceUsesOfWith on a constant!");

  for (unsigned i = 0, E = getNumOperands(); i != E; ++i)
    if (getOperand(i) == From) {  // Is This operand is pointing to oldval?
      // The side effects of this setOperand call include linking to
      // "To", adding "this" to the uses list of To, and
      // most importantly, removing "this" from the use list of "From".
      setOperand(i, To); // Fix it now...
    }
}

//===----------------------------------------------------------------------===//
//                         User allocHungoffUses Implementation
//===----------------------------------------------------------------------===//

Use *User::allocHungoffUses(unsigned N) const {
  // Allocate the array of Uses, followed by a pointer (with bottom bit set) to
  // the User.
  size_t size = N * sizeof(Use) + sizeof(Use::UserRef);
  Use *Begin = static_cast<Use*>(::operator new(size));
  Use *End = Begin + N;
  (void) new(End) Use::UserRef(const_cast<User*>(this), 1);
  return Use::initTags(Begin, End);
}

//===----------------------------------------------------------------------===//
//                         User operator new Implementations
//===----------------------------------------------------------------------===//

void *User::operator new(size_t s, unsigned Us) {
  void *Storage = ::operator new(s + sizeof(Use) * Us);
  Use *Start = static_cast<Use*>(Storage);
  Use *End = Start + Us;
  User *Obj = reinterpret_cast<User*>(End);
  Obj->OperandList = Start;
  Obj->NumOperands = Us;
  Use::initTags(Start, End);
  return Obj;
}

//===----------------------------------------------------------------------===//
//                         User operator delete Implementation
//===----------------------------------------------------------------------===//

void User::operator delete(void *Usr) {
  User *Start = static_cast<User*>(Usr);
  Use *Storage = static_cast<Use*>(Usr) - Start->NumOperands;
  // If there were hung-off uses, they will have been freed already and
  // NumOperands reset to 0, so here we just free the User itself.
  ::operator delete(Storage);
}

//===----------------------------------------------------------------------===//
//                             Operator Class
//===----------------------------------------------------------------------===//

Operator::~Operator() {
  llvm_unreachable("should never destroy an Operator");
}

} // End llvm namespace