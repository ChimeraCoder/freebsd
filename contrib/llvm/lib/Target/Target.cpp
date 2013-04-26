
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

#include "llvm-c/Target.h"
#include "llvm-c/Initialization.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassManager.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include <cstring>

using namespace llvm;

void llvm::initializeTarget(PassRegistry &Registry) {
  initializeDataLayoutPass(Registry);
  initializeTargetLibraryInfoPass(Registry);
}

void LLVMInitializeTarget(LLVMPassRegistryRef R) {
  initializeTarget(*unwrap(R));
}

LLVMTargetDataRef LLVMCreateTargetData(const char *StringRep) {
  return wrap(new DataLayout(StringRep));
}

void LLVMAddTargetData(LLVMTargetDataRef TD, LLVMPassManagerRef PM) {
  unwrap(PM)->add(new DataLayout(*unwrap(TD)));
}

void LLVMAddTargetLibraryInfo(LLVMTargetLibraryInfoRef TLI,
                              LLVMPassManagerRef PM) {
  unwrap(PM)->add(new TargetLibraryInfo(*unwrap(TLI)));
}

char *LLVMCopyStringRepOfTargetData(LLVMTargetDataRef TD) {
  std::string StringRep = unwrap(TD)->getStringRepresentation();
  return strdup(StringRep.c_str());
}

LLVMByteOrdering LLVMByteOrder(LLVMTargetDataRef TD) {
  return unwrap(TD)->isLittleEndian() ? LLVMLittleEndian : LLVMBigEndian;
}

unsigned LLVMPointerSize(LLVMTargetDataRef TD) {
  return unwrap(TD)->getPointerSize(0);
}

unsigned LLVMPointerSizeForAS(LLVMTargetDataRef TD, unsigned AS) {
  return unwrap(TD)->getPointerSize(AS);
}

LLVMTypeRef LLVMIntPtrType(LLVMTargetDataRef TD) {
  return wrap(unwrap(TD)->getIntPtrType(getGlobalContext()));
}

LLVMTypeRef LLVMIntPtrTypeForAS(LLVMTargetDataRef TD, unsigned AS) {
  return wrap(unwrap(TD)->getIntPtrType(getGlobalContext(), AS));
}

unsigned long long LLVMSizeOfTypeInBits(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getTypeSizeInBits(unwrap(Ty));
}

unsigned long long LLVMStoreSizeOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getTypeStoreSize(unwrap(Ty));
}

unsigned long long LLVMABISizeOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getTypeAllocSize(unwrap(Ty));
}

unsigned LLVMABIAlignmentOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getABITypeAlignment(unwrap(Ty));
}

unsigned LLVMCallFrameAlignmentOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getCallFrameTypeAlignment(unwrap(Ty));
}

unsigned LLVMPreferredAlignmentOfType(LLVMTargetDataRef TD, LLVMTypeRef Ty) {
  return unwrap(TD)->getPrefTypeAlignment(unwrap(Ty));
}

unsigned LLVMPreferredAlignmentOfGlobal(LLVMTargetDataRef TD,
                                        LLVMValueRef GlobalVar) {
  return unwrap(TD)->getPreferredAlignment(unwrap<GlobalVariable>(GlobalVar));
}

unsigned LLVMElementAtOffset(LLVMTargetDataRef TD, LLVMTypeRef StructTy,
                             unsigned long long Offset) {
  StructType *STy = unwrap<StructType>(StructTy);
  return unwrap(TD)->getStructLayout(STy)->getElementContainingOffset(Offset);
}

unsigned long long LLVMOffsetOfElement(LLVMTargetDataRef TD, LLVMTypeRef StructTy,
                                       unsigned Element) {
  StructType *STy = unwrap<StructType>(StructTy);
  return unwrap(TD)->getStructLayout(STy)->getElementOffset(Element);
}

void LLVMDisposeTargetData(LLVMTargetDataRef TD) {
  delete unwrap(TD);
}