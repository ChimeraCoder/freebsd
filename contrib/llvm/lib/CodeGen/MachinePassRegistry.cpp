
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

#include "llvm/CodeGen/MachinePassRegistry.h"

using namespace llvm;

void MachinePassRegistryListener::anchor() { }

/// setDefault - Set the default constructor by name.
void MachinePassRegistry::setDefault(StringRef Name) {
  MachinePassCtor Ctor = 0;
  for(MachinePassRegistryNode *R = getList(); R; R = R->getNext()) {
    if (R->getName() == Name) {
      Ctor = R->getCtor();
      break;
    }
  }
  assert(Ctor && "Unregistered pass name");
  setDefault(Ctor);
}

/// Add - Adds a function pass to the registration list.
///
void MachinePassRegistry::Add(MachinePassRegistryNode *Node) {
  Node->setNext(List);
  List = Node;
  if (Listener) Listener->NotifyAdd(Node->getName(),
                                    Node->getCtor(),
                                    Node->getDescription());
}


/// Remove - Removes a function pass from the registration list.
///
void MachinePassRegistry::Remove(MachinePassRegistryNode *Node) {
  for (MachinePassRegistryNode **I = &List; *I; I = (*I)->getNextAddress()) {
    if (*I == Node) {
      if (Listener) Listener->NotifyRemove(Node->getName());
      *I = (*I)->getNext();
      break;
    }
  }
}