
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

#include "RecordingMemoryManager.h"
using namespace llvm;

RecordingMemoryManager::~RecordingMemoryManager() {
  for (SmallVectorImpl<Allocation>::iterator
         I = AllocatedCodeMem.begin(), E = AllocatedCodeMem.end();
       I != E; ++I)
    sys::Memory::releaseMappedMemory(I->first);
  for (SmallVectorImpl<Allocation>::iterator
         I = AllocatedDataMem.begin(), E = AllocatedDataMem.end();
       I != E; ++I)
    sys::Memory::releaseMappedMemory(I->first);
}

uint8_t *RecordingMemoryManager::
allocateCodeSection(uintptr_t Size, unsigned Alignment, unsigned SectionID) {
  // The recording memory manager is just a local copy of the remote target.
  // The alignment requirement is just stored here for later use. Regular
  // heap storage is sufficient here, but we're using mapped memory to work
  // around a bug in MCJIT.
  sys::MemoryBlock Block = allocateSection(Size);
  AllocatedCodeMem.push_back(Allocation(Block, Alignment));
  return (uint8_t*)Block.base();
}

uint8_t *RecordingMemoryManager::
allocateDataSection(uintptr_t Size, unsigned Alignment,
                    unsigned SectionID, bool IsReadOnly) {
  // The recording memory manager is just a local copy of the remote target.
  // The alignment requirement is just stored here for later use. Regular
  // heap storage is sufficient here, but we're using mapped memory to work
  // around a bug in MCJIT.
  sys::MemoryBlock Block = allocateSection(Size);
  AllocatedDataMem.push_back(Allocation(Block, Alignment));
  return (uint8_t*)Block.base();
}

sys::MemoryBlock RecordingMemoryManager::allocateSection(uintptr_t Size) {
  error_code ec;
  sys::MemoryBlock MB = sys::Memory::allocateMappedMemory(Size,
                                                          &Near,
                                                          sys::Memory::MF_READ |
                                                          sys::Memory::MF_WRITE,
                                                          ec);
  assert(!ec && MB.base());

  // FIXME: This is part of a work around to keep sections near one another
  // when MCJIT performs relocations after code emission but before
  // the generated code is moved to the remote target.
  // Save this address as the basis for our next request
  Near = MB;
  return MB;
}

void RecordingMemoryManager::setMemoryWritable() { llvm_unreachable("Unexpected!"); }
void RecordingMemoryManager::setMemoryExecutable() { llvm_unreachable("Unexpected!"); }
void RecordingMemoryManager::setPoisonMemory(bool poison) { llvm_unreachable("Unexpected!"); }
void RecordingMemoryManager::AllocateGOT() { llvm_unreachable("Unexpected!"); }
uint8_t *RecordingMemoryManager::getGOTBase() const {
  llvm_unreachable("Unexpected!");
  return 0;
}
uint8_t *RecordingMemoryManager::startFunctionBody(const Function *F, uintptr_t &ActualSize){
  llvm_unreachable("Unexpected!");
  return 0;
}
uint8_t *RecordingMemoryManager::allocateStub(const GlobalValue* F, unsigned StubSize,
                                              unsigned Alignment) {
  llvm_unreachable("Unexpected!");
  return 0;
}
void RecordingMemoryManager::endFunctionBody(const Function *F, uint8_t *FunctionStart,
                                             uint8_t *FunctionEnd) {
  llvm_unreachable("Unexpected!");
}
uint8_t *RecordingMemoryManager::allocateSpace(intptr_t Size, unsigned Alignment) {
  llvm_unreachable("Unexpected!");
  return 0;
}
uint8_t *RecordingMemoryManager::allocateGlobal(uintptr_t Size, unsigned Alignment) {
  llvm_unreachable("Unexpected!");
  return 0;
}
void RecordingMemoryManager::deallocateFunctionBody(void *Body) {
  llvm_unreachable("Unexpected!");
}
uint8_t* RecordingMemoryManager::startExceptionTable(const Function* F, uintptr_t &ActualSize) {
  llvm_unreachable("Unexpected!");
  return 0;
}
void RecordingMemoryManager::endExceptionTable(const Function *F, uint8_t *TableStart,
                                               uint8_t *TableEnd, uint8_t* FrameRegister) {
  llvm_unreachable("Unexpected!");
}
void RecordingMemoryManager::deallocateExceptionTable(void *ET) {
  llvm_unreachable("Unexpected!");
}

static int jit_noop() {
  return 0;
}

void *RecordingMemoryManager::getPointerToNamedFunction(const std::string &Name,
                                                        bool AbortOnFailure) {
  // We should not invoke parent's ctors/dtors from generated main()!
  // On Mingw and Cygwin, the symbol __main is resolved to
  // callee's(eg. tools/lli) one, to invoke wrong duplicated ctors
  // (and register wrong callee's dtors with atexit(3)).
  // We expect ExecutionEngine::runStaticConstructorsDestructors()
  // is called before ExecutionEngine::runFunctionAsMain() is called.
  if (Name == "__main") return (void*)(intptr_t)&jit_noop;

  return NULL;
}