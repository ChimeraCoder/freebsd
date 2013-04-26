
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

#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

void BranchProbability::print(raw_ostream &OS) const {
  OS << N << " / " << D << " = " << format("%g%%", ((double)N / D) * 100.0);
}

void BranchProbability::dump() const {
  dbgs() << *this << '\n';
}

namespace llvm {

raw_ostream &operator<<(raw_ostream &OS, const BranchProbability &Prob) {
  Prob.print(OS);
  return OS;
}

}