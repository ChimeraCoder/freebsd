
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

#include "llvm/Config/config.h"

#if HAVE_ISNAN_IN_MATH_H
# include <math.h>
#elif HAVE_ISNAN_IN_CMATH
# include <cmath>
#elif HAVE_STD_ISNAN_IN_CMATH
# include <cmath>
using std::isnan;
#elif defined(_MSC_VER)
#include <float.h>
#define isnan _isnan
#else
# error "Don't know how to get isnan()"
#endif

namespace llvm {
  int IsNAN(float f)  { return isnan(f); }
  int IsNAN(double d) { return isnan(d); }
} // end namespace llvm;