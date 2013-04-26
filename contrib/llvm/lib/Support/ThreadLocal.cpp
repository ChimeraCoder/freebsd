
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

#include "llvm/Config/config.h"
#include "llvm/Support/ThreadLocal.h"

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#if !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled
namespace llvm {
using namespace sys;
ThreadLocalImpl::ThreadLocalImpl() { }
ThreadLocalImpl::~ThreadLocalImpl() { }
void ThreadLocalImpl::setInstance(const void* d) {
  typedef int SIZE_TOO_BIG[sizeof(d) <= sizeof(data) ? 1 : -1];
  void **pd = reinterpret_cast<void**>(&data);
  *pd = const_cast<void*>(d);
}
const void* ThreadLocalImpl::getInstance() {
  void **pd = reinterpret_cast<void**>(&data);
  return *pd;
}
void ThreadLocalImpl::removeInstance() {
  setInstance(0);
}
}
#else

#if defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_GETSPECIFIC)

#include <cassert>
#include <pthread.h>
#include <stdlib.h>

namespace llvm {
using namespace sys;

ThreadLocalImpl::ThreadLocalImpl() : data() {
  typedef int SIZE_TOO_BIG[sizeof(pthread_key_t) <= sizeof(data) ? 1 : -1];
  pthread_key_t* key = reinterpret_cast<pthread_key_t*>(&data);
  int errorcode = pthread_key_create(key, NULL);
  assert(errorcode == 0);
  (void) errorcode;
}

ThreadLocalImpl::~ThreadLocalImpl() {
  pthread_key_t* key = reinterpret_cast<pthread_key_t*>(&data);
  int errorcode = pthread_key_delete(*key);
  assert(errorcode == 0);
  (void) errorcode;
}

void ThreadLocalImpl::setInstance(const void* d) {
  pthread_key_t* key = reinterpret_cast<pthread_key_t*>(&data);
  int errorcode = pthread_setspecific(*key, d);
  assert(errorcode == 0);
  (void) errorcode;
}

const void* ThreadLocalImpl::getInstance() {
  pthread_key_t* key = reinterpret_cast<pthread_key_t*>(&data);
  return pthread_getspecific(*key);
}

void ThreadLocalImpl::removeInstance() {
  setInstance(0);
}

}

#elif defined(LLVM_ON_UNIX)
#include "Unix/ThreadLocal.inc"
#elif defined( LLVM_ON_WIN32)
#include "Windows/ThreadLocal.inc"
#else
#warning Neither LLVM_ON_UNIX nor LLVM_ON_WIN32 set in Support/ThreadLocal.cpp
#endif
#endif