
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

#include "AMDILIntrinsicInfo.h"
#include "AMDGPUSubtarget.h"
#include "AMDIL.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"

using namespace llvm;

#define GET_LLVM_INTRINSIC_FOR_GCC_BUILTIN
#include "AMDGPUGenIntrinsics.inc"
#undef GET_LLVM_INTRINSIC_FOR_GCC_BUILTIN

AMDGPUIntrinsicInfo::AMDGPUIntrinsicInfo(TargetMachine *tm) 
  : TargetIntrinsicInfo() {
}

std::string 
AMDGPUIntrinsicInfo::getName(unsigned int IntrID, Type **Tys,
    unsigned int numTys) const  {
  static const char* const names[] = {
#define GET_INTRINSIC_NAME_TABLE
#include "AMDGPUGenIntrinsics.inc"
#undef GET_INTRINSIC_NAME_TABLE
  };

  if (IntrID < Intrinsic::num_intrinsics) {
    return 0;
  }
  assert(IntrID < AMDGPUIntrinsic::num_AMDGPU_intrinsics
      && "Invalid intrinsic ID");

  std::string Result(names[IntrID - Intrinsic::num_intrinsics]);
  return Result;
}

unsigned int
AMDGPUIntrinsicInfo::lookupName(const char *Name, unsigned int Len) const  {
#define GET_FUNCTION_RECOGNIZER
#include "AMDGPUGenIntrinsics.inc"
#undef GET_FUNCTION_RECOGNIZER
  AMDGPUIntrinsic::ID IntrinsicID
    = (AMDGPUIntrinsic::ID)Intrinsic::not_intrinsic;
  IntrinsicID = getIntrinsicForGCCBuiltin("AMDGPU", Name);

  if (IntrinsicID != (AMDGPUIntrinsic::ID)Intrinsic::not_intrinsic) {
    return IntrinsicID;
  }
  return 0;
}

bool 
AMDGPUIntrinsicInfo::isOverloaded(unsigned id) const  {
  // Overload Table
#define GET_INTRINSIC_OVERLOAD_TABLE
#include "AMDGPUGenIntrinsics.inc"
#undef GET_INTRINSIC_OVERLOAD_TABLE
}

Function*
AMDGPUIntrinsicInfo::getDeclaration(Module *M, unsigned IntrID,
    Type **Tys,
    unsigned numTys) const  {
  llvm_unreachable("Not implemented");
}