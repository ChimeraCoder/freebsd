
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

#include "llvm/IR/TypeFinder.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
using namespace llvm;

void TypeFinder::run(const Module &M, bool onlyNamed) {
  OnlyNamed = onlyNamed;

  // Get types from global variables.
  for (Module::const_global_iterator I = M.global_begin(),
         E = M.global_end(); I != E; ++I) {
    incorporateType(I->getType());
    if (I->hasInitializer())
      incorporateValue(I->getInitializer());
  }

  // Get types from aliases.
  for (Module::const_alias_iterator I = M.alias_begin(),
         E = M.alias_end(); I != E; ++I) {
    incorporateType(I->getType());
    if (const Value *Aliasee = I->getAliasee())
      incorporateValue(Aliasee);
  }

  // Get types from functions.
  SmallVector<std::pair<unsigned, MDNode*>, 4> MDForInst;
  for (Module::const_iterator FI = M.begin(), E = M.end(); FI != E; ++FI) {
    incorporateType(FI->getType());

    // First incorporate the arguments.
    for (Function::const_arg_iterator AI = FI->arg_begin(),
           AE = FI->arg_end(); AI != AE; ++AI)
      incorporateValue(AI);

    for (Function::const_iterator BB = FI->begin(), E = FI->end();
         BB != E;++BB)
      for (BasicBlock::const_iterator II = BB->begin(),
             E = BB->end(); II != E; ++II) {
        const Instruction &I = *II;

        // Incorporate the type of the instruction.
        incorporateType(I.getType());

        // Incorporate non-instruction operand types. (We are incorporating all
        // instructions with this loop.)
        for (User::const_op_iterator OI = I.op_begin(), OE = I.op_end();
             OI != OE; ++OI)
          if (!isa<Instruction>(OI))
            incorporateValue(*OI);

        // Incorporate types hiding in metadata.
        I.getAllMetadataOtherThanDebugLoc(MDForInst);
        for (unsigned i = 0, e = MDForInst.size(); i != e; ++i)
          incorporateMDNode(MDForInst[i].second);

        MDForInst.clear();
      }
  }

  for (Module::const_named_metadata_iterator I = M.named_metadata_begin(),
         E = M.named_metadata_end(); I != E; ++I) {
    const NamedMDNode *NMD = I;
    for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i)
      incorporateMDNode(NMD->getOperand(i));
  }
}

void TypeFinder::clear() {
  VisitedConstants.clear();
  VisitedTypes.clear();
  StructTypes.clear();
}

/// incorporateType - This method adds the type to the list of used structures
/// if it's not in there already.
void TypeFinder::incorporateType(Type *Ty) {
  // Check to see if we're already visited this type.
  if (!VisitedTypes.insert(Ty).second)
    return;

  // If this is a structure or opaque type, add a name for the type.
  if (StructType *STy = dyn_cast<StructType>(Ty))
    if (!OnlyNamed || STy->hasName())
      StructTypes.push_back(STy);

  // Recursively walk all contained types.
  for (Type::subtype_iterator I = Ty->subtype_begin(),
         E = Ty->subtype_end(); I != E; ++I)
    incorporateType(*I);
}

/// incorporateValue - This method is used to walk operand lists finding types
/// hiding in constant expressions and other operands that won't be walked in
/// other ways.  GlobalValues, basic blocks, instructions, and inst operands are
/// all explicitly enumerated.
void TypeFinder::incorporateValue(const Value *V) {
  if (const MDNode *M = dyn_cast<MDNode>(V))
    return incorporateMDNode(M);

  if (!isa<Constant>(V) || isa<GlobalValue>(V)) return;

  // Already visited?
  if (!VisitedConstants.insert(V).second)
    return;

  // Check this type.
  incorporateType(V->getType());

  // If this is an instruction, we incorporate it separately.
  if (isa<Instruction>(V))
    return;

  // Look in operands for types.
  const User *U = cast<User>(V);
  for (Constant::const_op_iterator I = U->op_begin(),
         E = U->op_end(); I != E;++I)
    incorporateValue(*I);
}

/// incorporateMDNode - This method is used to walk the operands of an MDNode to
/// find types hiding within.
void TypeFinder::incorporateMDNode(const MDNode *V) {
  // Already visited?
  if (!VisitedConstants.insert(V).second)
    return;

  // Look in operands for types.
  for (unsigned i = 0, e = V->getNumOperands(); i != e; ++i)
    if (Value *Op = V->getOperand(i))
      incorporateValue(Op);
}