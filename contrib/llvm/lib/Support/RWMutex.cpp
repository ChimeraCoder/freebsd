
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
#include "llvm/Support/RWMutex.h"
#include <cstring>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#if !defined(LLVM_ENABLE_THREADS) || LLVM_ENABLE_THREADS == 0
// Define all methods as no-ops if threading is explicitly disabled
namespace llvm {
using namespace sys;
RWMutexImpl::RWMutexImpl() { }
RWMutexImpl::~RWMutexImpl() { }
bool RWMutexImpl::reader_acquire() { return true; }
bool RWMutexImpl::reader_release() { return true; }
bool RWMutexImpl::writer_acquire() { return true; }
bool RWMutexImpl::writer_release() { return true; }
}
#else

#if defined(HAVE_PTHREAD_H) && defined(HAVE_PTHREAD_RWLOCK_INIT)

#include <cassert>
#include <pthread.h>
#include <stdlib.h>

namespace llvm {
using namespace sys;

// Construct a RWMutex using pthread calls
RWMutexImpl::RWMutexImpl()
  : data_(0)
{
  // Declare the pthread_rwlock data structures
  pthread_rwlock_t* rwlock =
    static_cast<pthread_rwlock_t*>(malloc(sizeof(pthread_rwlock_t)));

#ifdef __APPLE__
  // Workaround a bug/mis-feature in Darwin's pthread_rwlock_init.
  bzero(rwlock, sizeof(pthread_rwlock_t));
#endif

  // Initialize the rwlock
  int errorcode = pthread_rwlock_init(rwlock, NULL);
  (void)errorcode;
  assert(errorcode == 0);

  // Assign the data member
  data_ = rwlock;
}

// Destruct a RWMutex
RWMutexImpl::~RWMutexImpl()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != 0);
  pthread_rwlock_destroy(rwlock);
  free(rwlock);
}

bool
RWMutexImpl::reader_acquire()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != 0);

  int errorcode = pthread_rwlock_rdlock(rwlock);
  return errorcode == 0;
}

bool
RWMutexImpl::reader_release()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != 0);

  int errorcode = pthread_rwlock_unlock(rwlock);
  return errorcode == 0;
}

bool
RWMutexImpl::writer_acquire()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != 0);

  int errorcode = pthread_rwlock_wrlock(rwlock);
  return errorcode == 0;
}

bool
RWMutexImpl::writer_release()
{
  pthread_rwlock_t* rwlock = static_cast<pthread_rwlock_t*>(data_);
  assert(rwlock != 0);

  int errorcode = pthread_rwlock_unlock(rwlock);
  return errorcode == 0;
}

}

#elif defined(LLVM_ON_UNIX)
#include "Unix/RWMutex.inc"
#elif defined( LLVM_ON_WIN32)
#include "Windows/RWMutex.inc"
#else
#warning Neither LLVM_ON_UNIX nor LLVM_ON_WIN32 was set in Support/Mutex.cpp
#endif
#endif