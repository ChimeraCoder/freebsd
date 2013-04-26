
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

#include "CGOpenCLRuntime.h"
#include "CodeGenFunction.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include <assert.h>

using namespace clang;
using namespace CodeGen;

CGOpenCLRuntime::~CGOpenCLRuntime() {}

void CGOpenCLRuntime::EmitWorkGroupLocalVarDecl(CodeGenFunction &CGF,
                                                const VarDecl &D) {
  return CGF.EmitStaticVarDecl(D, llvm::GlobalValue::InternalLinkage);
}

llvm::Type *CGOpenCLRuntime::convertOpenCLSpecificType(const Type *T) {
  assert(T->isOpenCLSpecificType() &&
         "Not an OpenCL specific type!");

  switch (cast<BuiltinType>(T)->getKind()) {
  default: 
    llvm_unreachable("Unexpected opencl builtin type!");
    return 0;
  case BuiltinType::OCLImage1d:
    return llvm::PointerType::get(llvm::StructType::create(
                           CGM.getLLVMContext(), "opencl.image1d_t"), 0);
  case BuiltinType::OCLImage1dArray:
    return llvm::PointerType::get(llvm::StructType::create(
                           CGM.getLLVMContext(), "opencl.image1d_array_t"), 0);
  case BuiltinType::OCLImage1dBuffer:
    return llvm::PointerType::get(llvm::StructType::create(
                           CGM.getLLVMContext(), "opencl.image1d_buffer_t"), 0);
  case BuiltinType::OCLImage2d:
    return llvm::PointerType::get(llvm::StructType::create(
                           CGM.getLLVMContext(), "opencl.image2d_t"), 0);
  case BuiltinType::OCLImage2dArray:
    return llvm::PointerType::get(llvm::StructType::create(
                           CGM.getLLVMContext(), "opencl.image2d_array_t"), 0);
  case BuiltinType::OCLImage3d:
    return llvm::PointerType::get(llvm::StructType::create(
                           CGM.getLLVMContext(), "opencl.image3d_t"), 0);
  case BuiltinType::OCLSampler:
    return llvm::IntegerType::get(CGM.getLLVMContext(),32);
  case BuiltinType::OCLEvent:
    return llvm::PointerType::get(llvm::StructType::create(
                           CGM.getLLVMContext(), "opencl.event_t"), 0);
  }
}