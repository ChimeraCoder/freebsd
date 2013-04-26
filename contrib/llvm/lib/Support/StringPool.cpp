
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

#include "llvm/Support/StringPool.h"
#include "llvm/ADT/StringRef.h"

using namespace llvm;

StringPool::StringPool() {}

StringPool::~StringPool() {
  assert(InternTable.empty() && "PooledStringPtr leaked!");
}

PooledStringPtr StringPool::intern(StringRef Key) {
  table_t::iterator I = InternTable.find(Key);
  if (I != InternTable.end())
    return PooledStringPtr(&*I);
  
  entry_t *S = entry_t::Create(Key.begin(), Key.end());
  S->getValue().Pool = this;
  InternTable.insert(S);
  
  return PooledStringPtr(S);
}