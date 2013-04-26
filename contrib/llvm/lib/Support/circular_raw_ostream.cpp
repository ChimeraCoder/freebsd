
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

#include "llvm/Support/circular_raw_ostream.h"
#include <algorithm>
using namespace llvm;

void circular_raw_ostream::write_impl(const char *Ptr, size_t Size) {
  if (BufferSize == 0) {
    TheStream->write(Ptr, Size);
    return;
  }

  // Write into the buffer, wrapping if necessary.
  while (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    if (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = true;
    }
  }    
}

void circular_raw_ostream::flushBufferWithBanner() {
  if (BufferSize != 0) {
    // Write out the buffer
    TheStream->write(Banner, std::strlen(Banner));
    flushBuffer();
  }
}