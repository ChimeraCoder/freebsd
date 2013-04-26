
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

#include "llvm/Support/ManagedStatic.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Atomic.h"
#include <cassert>
using namespace llvm;

static const ManagedStaticBase *StaticList = 0;

void ManagedStaticBase::RegisterManagedStatic(void *(*Creator)(),
                                              void (*Deleter)(void*)) const {
  if (llvm_is_multithreaded()) {
    llvm_acquire_global_lock();

    if (Ptr == 0) {
      void* tmp = Creator ? Creator() : 0;

      TsanHappensBefore(this);
      sys::MemoryFence();

      // This write is racy against the first read in the ManagedStatic
      // accessors. The race is benign because it does a second read after a
      // memory fence, at which point it isn't possible to get a partial value.
      TsanIgnoreWritesBegin();
      Ptr = tmp;
      TsanIgnoreWritesEnd();
      DeleterFn = Deleter;
      
      // Add to list of managed statics.
      Next = StaticList;
      StaticList = this;
    }

    llvm_release_global_lock();
  } else {
    assert(Ptr == 0 && DeleterFn == 0 && Next == 0 &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator ? Creator() : 0;
    DeleterFn = Deleter;
  
    // Add to list of managed statics.
    Next = StaticList;
    StaticList = this;
  }
}

void ManagedStaticBase::destroy() const {
  assert(DeleterFn && "ManagedStatic not initialized correctly!");
  assert(StaticList == this &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = Next;
  Next = 0;

  // Destroy memory.
  DeleterFn(Ptr);
  
  // Cleanup.
  Ptr = 0;
  DeleterFn = 0;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
void llvm::llvm_shutdown() {
  while (StaticList)
    StaticList->destroy();

  if (llvm_is_multithreaded()) llvm_stop_multithreaded();
}