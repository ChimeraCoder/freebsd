
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

#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
using namespace llvm;

namespace {
struct PSVGlobalsTy {
  // PseudoSourceValues are immutable so don't need locking.
  const PseudoSourceValue PSVs[4];
  sys::Mutex Lock;  // Guards FSValues, but not the values inside it.
  std::map<int, const PseudoSourceValue *> FSValues;

  PSVGlobalsTy() : PSVs() {}
  ~PSVGlobalsTy() {
    for (std::map<int, const PseudoSourceValue *>::iterator
           I = FSValues.begin(), E = FSValues.end(); I != E; ++I) {
      delete I->second;
    }
  }
};

static ManagedStatic<PSVGlobalsTy> PSVGlobals;

}  // anonymous namespace

const PseudoSourceValue *PseudoSourceValue::getStack()
{ return &PSVGlobals->PSVs[0]; }
const PseudoSourceValue *PseudoSourceValue::getGOT()
{ return &PSVGlobals->PSVs[1]; }
const PseudoSourceValue *PseudoSourceValue::getJumpTable()
{ return &PSVGlobals->PSVs[2]; }
const PseudoSourceValue *PseudoSourceValue::getConstantPool()
{ return &PSVGlobals->PSVs[3]; }

static const char *const PSVNames[] = {
  "Stack",
  "GOT",
  "JumpTable",
  "ConstantPool"
};

// FIXME: THIS IS A HACK!!!!
// Eventually these should be uniqued on LLVMContext rather than in a managed
// static.  For now, we can safely use the global context for the time being to
// squeak by.
PseudoSourceValue::PseudoSourceValue(enum ValueTy Subclass) :
  Value(Type::getInt8PtrTy(getGlobalContext()),
        Subclass) {}

void PseudoSourceValue::printCustom(raw_ostream &O) const {
  O << PSVNames[this - PSVGlobals->PSVs];
}

const PseudoSourceValue *PseudoSourceValue::getFixedStack(int FI) {
  PSVGlobalsTy &PG = *PSVGlobals;
  sys::ScopedLock locked(PG.Lock);
  const PseudoSourceValue *&V = PG.FSValues[FI];
  if (!V)
    V = new FixedStackPseudoSourceValue(FI);
  return V;
}

bool PseudoSourceValue::isConstant(const MachineFrameInfo *) const {
  if (this == getStack())
    return false;
  if (this == getGOT() ||
      this == getConstantPool() ||
      this == getJumpTable())
    return true;
  llvm_unreachable("Unknown PseudoSourceValue!");
}

bool PseudoSourceValue::isAliased(const MachineFrameInfo *MFI) const {
  if (this == getStack() ||
      this == getGOT() ||
      this == getConstantPool() ||
      this == getJumpTable())
    return false;
  llvm_unreachable("Unknown PseudoSourceValue!");
}

bool PseudoSourceValue::mayAlias(const MachineFrameInfo *MFI) const {
  if (this == getGOT() ||
      this == getConstantPool() ||
      this == getJumpTable())
    return false;
  return true;
}

bool FixedStackPseudoSourceValue::isConstant(const MachineFrameInfo *MFI) const{
  return MFI && MFI->isImmutableObjectIndex(FI);
}

bool FixedStackPseudoSourceValue::isAliased(const MachineFrameInfo *MFI) const {
  // Negative frame indices are used for special things that don't
  // appear in LLVM IR. Non-negative indices may be used for things
  // like static allocas.
  if (!MFI)
    return FI >= 0;
  // Spill slots should not alias others.
  return !MFI->isFixedObjectIndex(FI) && !MFI->isSpillSlotObjectIndex(FI);
}

bool FixedStackPseudoSourceValue::mayAlias(const MachineFrameInfo *MFI) const {
  if (!MFI)
    return true;
  // Spill slots will not alias any LLVM IR value.
  return !MFI->isSpillSlotObjectIndex(FI);
}

void FixedStackPseudoSourceValue::printCustom(raw_ostream &OS) const {
  OS << "FixedStack" << FI;
}