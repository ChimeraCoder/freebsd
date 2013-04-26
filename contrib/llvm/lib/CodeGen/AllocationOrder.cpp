
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

#define DEBUG_TYPE "regalloc"
#include "AllocationOrder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterClassInfo.h"
#include "llvm/CodeGen/VirtRegMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Compare VirtRegMap::getRegAllocPref().
AllocationOrder::AllocationOrder(unsigned VirtReg,
                                 const VirtRegMap &VRM,
                                 const RegisterClassInfo &RegClassInfo)
  : Pos(0) {
  const MachineFunction &MF = VRM.getMachineFunction();
  const TargetRegisterInfo *TRI = &VRM.getTargetRegInfo();
  Order = RegClassInfo.getOrder(MF.getRegInfo().getRegClass(VirtReg));
  TRI->getRegAllocationHints(VirtReg, Order, Hints, MF, &VRM);
  rewind();

  DEBUG({
    if (!Hints.empty()) {
      dbgs() << "hints:";
      for (unsigned I = 0, E = Hints.size(); I != E; ++I)
        dbgs() << ' ' << PrintReg(Hints[I], TRI);
      dbgs() << '\n';
    }
  });
#ifndef NDEBUG
  for (unsigned I = 0, E = Hints.size(); I != E; ++I)
    assert(std::find(Order.begin(), Order.end(), Hints[I]) != Order.end() &&
           "Target hint is outside allocation order.");
#endif
}