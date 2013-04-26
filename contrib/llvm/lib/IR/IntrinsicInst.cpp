
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

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Metadata.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
/// DbgInfoIntrinsic - This is the common base class for debug info intrinsics
///

static Value *CastOperand(Value *C) {
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(C))
    if (CE->isCast())
      return CE->getOperand(0);
  return NULL;
}

Value *DbgInfoIntrinsic::StripCast(Value *C) {
  if (Value *CO = CastOperand(C)) {
    C = StripCast(CO);
  } else if (GlobalVariable *GV = dyn_cast<GlobalVariable>(C)) {
    if (GV->hasInitializer())
      if (Value *CO = CastOperand(GV->getInitializer()))
        C = StripCast(CO);
  }
  return dyn_cast<GlobalVariable>(C);
}

//===----------------------------------------------------------------------===//
/// DbgDeclareInst - This represents the llvm.dbg.declare instruction.
///

Value *DbgDeclareInst::getAddress() const {
  if (MDNode* MD = cast_or_null<MDNode>(getArgOperand(0)))
    return MD->getOperand(0);
  else
    return NULL;
}

//===----------------------------------------------------------------------===//
/// DbgValueInst - This represents the llvm.dbg.value instruction.
///

const Value *DbgValueInst::getValue() const {
  return cast<MDNode>(getArgOperand(0))->getOperand(0);
}

Value *DbgValueInst::getValue() {
  return cast<MDNode>(getArgOperand(0))->getOperand(0);
}