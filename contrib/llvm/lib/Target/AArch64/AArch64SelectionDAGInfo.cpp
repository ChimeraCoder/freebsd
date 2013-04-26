
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

#define DEBUG_TYPE "arm-selectiondag-info"
#include "AArch64TargetMachine.h"
#include "llvm/CodeGen/SelectionDAG.h"
using namespace llvm;

AArch64SelectionDAGInfo::AArch64SelectionDAGInfo(const AArch64TargetMachine &TM)
  : TargetSelectionDAGInfo(TM),
    Subtarget(&TM.getSubtarget<AArch64Subtarget>()) {
}

AArch64SelectionDAGInfo::~AArch64SelectionDAGInfo() {
}