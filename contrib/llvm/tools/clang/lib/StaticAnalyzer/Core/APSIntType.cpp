
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

#include "clang/StaticAnalyzer/Core/PathSensitive/APSIntType.h"

using namespace clang;
using namespace ento;

APSIntType::RangeTestResultKind
APSIntType::testInRange(const llvm::APSInt &Value,
                        bool AllowSignConversions) const {

  // Negative numbers cannot be losslessly converted to unsigned type.
  if (IsUnsigned && !AllowSignConversions &&
      Value.isSigned() && Value.isNegative())
    return RTR_Below;

  unsigned MinBits;
  if (AllowSignConversions) {
    if (Value.isSigned() && !IsUnsigned)
      MinBits = Value.getMinSignedBits();
    else
      MinBits = Value.getActiveBits();

  } else {
    // Signed integers can be converted to signed integers of the same width
    // or (if positive) unsigned integers with one fewer bit.
    // Unsigned integers can be converted to unsigned integers of the same width
    // or signed integers with one more bit.
    if (Value.isSigned())
      MinBits = Value.getMinSignedBits() - IsUnsigned;
    else
      MinBits = Value.getActiveBits() + !IsUnsigned;
  }

  if (MinBits <= BitWidth)
    return RTR_Within;

  if (Value.isSigned() && Value.isNegative())
    return RTR_Below;
  else
    return RTR_Above;
}