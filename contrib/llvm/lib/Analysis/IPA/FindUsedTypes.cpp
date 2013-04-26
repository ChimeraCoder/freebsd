
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

#include "llvm/Analysis/FindUsedTypes.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

char FindUsedTypes::ID = 0;
INITIALIZE_PASS(FindUsedTypes, "print-used-types",
                "Find Used Types", false, true)

// IncorporateType - Incorporate one type and all of its subtypes into the
// collection of used types.
//
void FindUsedTypes::IncorporateType(Type *Ty) {
  // If ty doesn't already exist in the used types map, add it now, otherwise
  // return.
  if (!UsedTypes.insert(Ty)) return;  // Already contain Ty.

  // Make sure to add any types this type references now.
  //
  for (Type::subtype_iterator I = Ty->subtype_begin(), E = Ty->subtype_end();
       I != E; ++I)
    IncorporateType(*I);
}

void FindUsedTypes::IncorporateValue(const Value *V) {
  IncorporateType(V->getType());

  // If this is a constant, it could be using other types...
  if (const Constant *C = dyn_cast<Constant>(V)) {
    if (!isa<GlobalValue>(C))
      for (User::const_op_iterator OI = C->op_begin(), OE = C->op_end();
           OI != OE; ++OI)
        IncorporateValue(*OI);
  }
}


// run - This incorporates all types used by the specified module
//
bool FindUsedTypes::runOnModule(Module &m) {
  UsedTypes.clear();  // reset if run multiple times...

  // Loop over global variables, incorporating their types
  for (Module::const_global_iterator I = m.global_begin(), E = m.global_end();
       I != E; ++I) {
    IncorporateType(I->getType());
    if (I->hasInitializer())
      IncorporateValue(I->getInitializer());
  }

  for (Module::iterator MI = m.begin(), ME = m.end(); MI != ME; ++MI) {
    IncorporateType(MI->getType());
    const Function &F = *MI;

    // Loop over all of the instructions in the function, adding their return
    // type as well as the types of their operands.
    //
    for (const_inst_iterator II = inst_begin(F), IE = inst_end(F);
         II != IE; ++II) {
      const Instruction &I = *II;

      IncorporateType(I.getType());  // Incorporate the type of the instruction
      for (User::const_op_iterator OI = I.op_begin(), OE = I.op_end();
           OI != OE; ++OI)
        IncorporateValue(*OI);  // Insert inst operand types as well
    }
  }

  return false;
}

// Print the types found in the module.  If the optional Module parameter is
// passed in, then the types are printed symbolically if possible, using the
// symbol table from the module.
//
void FindUsedTypes::print(raw_ostream &OS, const Module *M) const {
  OS << "Types in use by this module:\n";
  for (SetVector<Type *>::const_iterator I = UsedTypes.begin(),
       E = UsedTypes.end(); I != E; ++I) {
    OS << "   " << **I << '\n';
  }
}