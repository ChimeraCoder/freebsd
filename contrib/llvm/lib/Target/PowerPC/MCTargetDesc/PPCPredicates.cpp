
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

#include "PPCPredicates.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
using namespace llvm;

PPC::Predicate PPC::InvertPredicate(PPC::Predicate Opcode) {
  switch (Opcode) {
  case PPC::PRED_EQ: return PPC::PRED_NE;
  case PPC::PRED_NE: return PPC::PRED_EQ;
  case PPC::PRED_LT: return PPC::PRED_GE;
  case PPC::PRED_GE: return PPC::PRED_LT;
  case PPC::PRED_GT: return PPC::PRED_LE;
  case PPC::PRED_LE: return PPC::PRED_GT;
  case PPC::PRED_NU: return PPC::PRED_UN;
  case PPC::PRED_UN: return PPC::PRED_NU;
  }
  llvm_unreachable("Unknown PPC branch opcode!");
}