
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

#include "llvm/MC/MCAtom.h"
#include "llvm/MC/MCModule.h"

using namespace llvm;

MCAtom *MCModule::createAtom(MCAtom::AtomType Type,
                             uint64_t Begin, uint64_t End) {
  assert(Begin < End && "Creating MCAtom with endpoints reversed?");

  // Check for atoms already covering this range.
  IntervalMap<uint64_t, MCAtom*>::iterator I = OffsetMap.find(Begin);
  assert((!I.valid() || I.start() < End) && "Offset range already occupied!");

  // Create the new atom and add it to our maps.
  MCAtom *NewAtom = new MCAtom(Type, this, Begin, End);
  AtomAllocationTracker.insert(NewAtom);
  OffsetMap.insert(Begin, End, NewAtom);
  return NewAtom;
}

// remap - Update the interval mapping for an atom.
void MCModule::remap(MCAtom *Atom, uint64_t NewBegin, uint64_t NewEnd) {
  // Find and erase the old mapping.
  IntervalMap<uint64_t, MCAtom*>::iterator I = OffsetMap.find(Atom->Begin);
  assert(I.valid() && "Atom offset not found in module!");
  assert(*I == Atom && "Previous atom mapping was invalid!");
  I.erase();

  // Insert the new mapping.
  OffsetMap.insert(NewBegin, NewEnd, Atom);

  // Update the atom internal bounds.
  Atom->Begin = NewBegin;
  Atom->End = NewEnd;
}