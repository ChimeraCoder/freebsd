
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

#define DEBUG_TYPE "hexagon-selectiondag-info"
#include "HexagonTargetMachine.h"
using namespace llvm;

bool llvm::flag_aligned_memcpy;

HexagonSelectionDAGInfo::HexagonSelectionDAGInfo(const HexagonTargetMachine
                                                 &TM)
  : TargetSelectionDAGInfo(TM) {
}

HexagonSelectionDAGInfo::~HexagonSelectionDAGInfo() {
}

SDValue
HexagonSelectionDAGInfo::
EmitTargetCodeForMemcpy(SelectionDAG &DAG, DebugLoc dl, SDValue Chain,
                        SDValue Dst, SDValue Src, SDValue Size, unsigned Align,
                        bool isVolatile, bool AlwaysInline,
                        MachinePointerInfo DstPtrInfo,
                        MachinePointerInfo SrcPtrInfo) const {
  flag_aligned_memcpy = false;
  if ((Align & 0x3) == 0) {
    ConstantSDNode *ConstantSize = dyn_cast<ConstantSDNode>(Size);
    if (ConstantSize) {
      uint64_t SizeVal = ConstantSize->getZExtValue();
      if ((SizeVal > 32) && ((SizeVal % 8) == 0))
        flag_aligned_memcpy = true;
    }
  }

  return SDValue();
}