
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

#include "llvm/Analysis/PtrUseVisitor.h"

using namespace llvm;

void detail::PtrUseVisitorBase::enqueueUsers(Instruction &I) {
  for (Value::use_iterator UI = I.use_begin(), UE = I.use_end();
       UI != UE; ++UI) {
    if (VisitedUses.insert(&UI.getUse())) {
      UseToVisit NewU = {
        UseToVisit::UseAndIsOffsetKnownPair(&UI.getUse(), IsOffsetKnown),
        Offset
      };
      Worklist.push_back(llvm_move(NewU));
    }
  }
}

bool detail::PtrUseVisitorBase::adjustOffsetForGEP(GetElementPtrInst &GEPI) {
  if (!IsOffsetKnown)
    return false;

  return GEPI.accumulateConstantOffset(DL, Offset);
}