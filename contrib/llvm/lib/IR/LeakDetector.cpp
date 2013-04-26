
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

#include "llvm/Support/LeakDetector.h"
#include "LLVMContextImpl.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Mutex.h"
#include "llvm/Support/Threading.h"
using namespace llvm;

static ManagedStatic<sys::SmartMutex<true> > ObjectsLock;
static ManagedStatic<LeakDetectorImpl<void> > Objects;

static void clearGarbage(LLVMContext &Context) {
  Objects->clear();
  Context.pImpl->LLVMObjects.clear();
}

void LeakDetector::addGarbageObjectImpl(void *Object) {
  sys::SmartScopedLock<true> Lock(*ObjectsLock);
  Objects->addGarbage(Object);
}

void LeakDetector::addGarbageObjectImpl(const Value *Object) {
  LLVMContextImpl *pImpl = Object->getContext().pImpl;
  pImpl->LLVMObjects.addGarbage(Object);
}

void LeakDetector::removeGarbageObjectImpl(void *Object) {
  sys::SmartScopedLock<true> Lock(*ObjectsLock);
  Objects->removeGarbage(Object);
}

void LeakDetector::removeGarbageObjectImpl(const Value *Object) {
  LLVMContextImpl *pImpl = Object->getContext().pImpl;
  pImpl->LLVMObjects.removeGarbage(Object);
}

void LeakDetector::checkForGarbageImpl(LLVMContext &Context, 
                                       const std::string &Message) {
  LLVMContextImpl *pImpl = Context.pImpl;
  sys::SmartScopedLock<true> Lock(*ObjectsLock);
  
  Objects->setName("GENERIC");
  pImpl->LLVMObjects.setName("LLVM");
  
  // use non-short-circuit version so that both checks are performed
  if (Objects->hasGarbage(Message) |
      pImpl->LLVMObjects.hasGarbage(Message))
    errs() << "\nThis is probably because you removed an object, but didn't "
           << "delete it.  Please check your code for memory leaks.\n";

  // Clear out results so we don't get duplicate warnings on
  // next call...
  clearGarbage(Context);
}