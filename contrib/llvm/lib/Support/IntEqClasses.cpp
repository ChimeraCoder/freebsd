
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

#include "llvm/ADT/IntEqClasses.h"

using namespace llvm;

void IntEqClasses::grow(unsigned N) {
  assert(NumClasses == 0 && "grow() called after compress().");
  EC.reserve(N);
  while (EC.size() < N)
    EC.push_back(EC.size());
}

void IntEqClasses::join(unsigned a, unsigned b) {
  assert(NumClasses == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  while (eca != ecb)
    if (eca < ecb)
      EC[b] = eca, b = ecb, ecb = EC[b];
    else
      EC[a] = ecb, a = eca, eca = EC[a];
}

unsigned IntEqClasses::findLeader(unsigned a) const {
  assert(NumClasses == 0 && "findLeader() called after compress().");
  while (a != EC[a])
    a = EC[a];
  return a;
}

void IntEqClasses::compress() {
  if (NumClasses)
    return;
  for (unsigned i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? NumClasses++ : EC[EC[i]];
}

void IntEqClasses::uncompress() {
  if (!NumClasses)
    return;
  SmallVector<unsigned, 8> Leader;
  for (unsigned i = 0, e = EC.size(); i != e; ++i)
    if (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    else
      Leader.push_back(EC[i] = i);
  NumClasses = 0;
}