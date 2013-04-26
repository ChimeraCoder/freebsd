
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

#include "RemoteTarget.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Memory.h"
#include <stdlib.h>
#include <string>
using namespace llvm;

bool RemoteTarget::allocateSpace(size_t Size, unsigned Alignment,
                                 uint64_t &Address) {
  sys::MemoryBlock *Prev = Allocations.size() ? &Allocations.back() : NULL;
  sys::MemoryBlock Mem = sys::Memory::AllocateRWX(Size, Prev, &ErrorMsg);
  if (Mem.base() == NULL)
    return true;
  if ((uintptr_t)Mem.base() % Alignment) {
    ErrorMsg = "unable to allocate sufficiently aligned memory";
    return true;
  }
  Address = reinterpret_cast<uint64_t>(Mem.base());
  return false;
}

bool RemoteTarget::loadData(uint64_t Address, const void *Data, size_t Size) {
  memcpy ((void*)Address, Data, Size);
  return false;
}

bool RemoteTarget::loadCode(uint64_t Address, const void *Data, size_t Size) {
  memcpy ((void*)Address, Data, Size);
  sys::MemoryBlock Mem((void*)Address, Size);
  sys::Memory::setExecutable(Mem, &ErrorMsg);
  return false;
}

bool RemoteTarget::executeCode(uint64_t Address, int &RetVal) {
  int (*fn)(void) = (int(*)(void))Address;
  RetVal = fn();
  return false;
}

void RemoteTarget::create() {
}

void RemoteTarget::stop() {
  for (unsigned i = 0, e = Allocations.size(); i != e; ++i)
    sys::Memory::ReleaseRWX(Allocations[i]);
}