
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

#include "llvm/Support/MemoryObject.h"
using namespace llvm;
  
MemoryObject::~MemoryObject() {
}

int MemoryObject::readBytes(uint64_t address,
                            uint64_t size,
                            uint8_t* buf,
                            uint64_t* copied) const {
  uint64_t current = address;
  uint64_t limit = getBase() + getExtent();

  if (current + size > limit)
    return -1;

  while (current - address < size) {
    if (readByte(current, &buf[(current - address)]))
      return -1;
    
    current++;
  }
  
  if (copied)
    *copied = current - address;
  
  return 0;
}