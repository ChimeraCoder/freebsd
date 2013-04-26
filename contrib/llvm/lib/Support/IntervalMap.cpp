
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

#include "llvm/ADT/IntervalMap.h"

namespace llvm {
namespace IntervalMapImpl {

void Path::replaceRoot(void *Root, unsigned Size, IdxPair Offsets) {
  assert(!path.empty() && "Can't replace missing root");
  path.front() = Entry(Root, Size, Offsets.first);
  path.insert(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

NodeRef Path::getLeftSibling(unsigned Level) const {
  // The root has no siblings.
  if (Level == 0)
    return NodeRef();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  while (l && path[l].offset == 0)
    --l;

  // We can't go left.
  if (path[l].offset == 0)
    return NodeRef();

  // NR is the subtree containing our left sibling.
  NodeRef NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  for (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  return NR;
}

void Path::moveLeft(unsigned Level) {
  assert(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  if (valid()) {
    l = Level - 1;
    while (path[l].offset == 0) {
      assert(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } else if (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(0, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  NodeRef NR = subtree(l);

  // Get the rightmost node in the subtree.
  for (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

NodeRef Path::getRightSibling(unsigned Level) const {
  // The root has no siblings.
  if (Level == 0)
    return NodeRef();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  while (l && atLastEntry(l))
    --l;

  // We can't go right.
  if (atLastEntry(l))
    return NodeRef();

  // NR is the subtree containing our right sibling.
  NodeRef NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  for (++l; l != Level; ++l)
    NR = NR.subtree(0);
  return NR;
}

void Path::moveRight(unsigned Level) {
  assert(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  while (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  if (++path[l].offset == path[l].size)
    return;
  NodeRef NR = subtree(l);

  for (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   const unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, bool Grow) {
  assert(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  assert(Position <= Elements && "Invalid position");
  if (!Nodes)
    return IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  const unsigned PerNode = (Elements + Grow) / Nodes;
  const unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  for (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = PerNode + (n < Extra);
    if (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  assert(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  if (Grow) {
    assert(PosPair.first < Nodes && "Bad algebra");
    assert(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  for (unsigned n = 0; n != Nodes; ++n) {
    assert(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  assert(Sum == Elements && "Bad distribution sum");
#endif

  return PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm