
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
#include "clang/Basic/VersionTuple.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

std::string VersionTuple::getAsString() const {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << *this;
  }
  return Result;
}

raw_ostream& clang::operator<<(raw_ostream &Out, 
                                     const VersionTuple &V) {
  Out << V.getMajor();
  if (Optional<unsigned> Minor = V.getMinor())
    Out << '.' << *Minor;
  if (Optional<unsigned> Subminor = V.getSubminor())
    Out << '.' << *Subminor;
  return Out;
}

static bool parseInt(StringRef &input, unsigned &value) {
  assert(value == 0);
  if (input.empty()) return true;

  char next = input[0];
  input = input.substr(1);
  if (next < '0' || next > '9') return true;
  value = (unsigned) (next - '0');

  while (!input.empty()) {
    next = input[0];
    if (next < '0' || next > '9') return false;
    input = input.substr(1);
    value = value * 10 + (unsigned) (next - '0');
  }

  return false;
}

bool VersionTuple::tryParse(StringRef input) {
  unsigned major = 0, minor = 0, micro = 0;

  // Parse the major version, [0-9]+
  if (parseInt(input, major)) return true;

  if (input.empty()) {
    *this = VersionTuple(major);
    return false;
  }

  // If we're not done, parse the minor version, \.[0-9]+
  if (input[0] != '.') return true;
  input = input.substr(1);
  if (parseInt(input, minor)) return true;

  if (input.empty()) {
    *this = VersionTuple(major, minor);
    return false;
  }

  // If we're not done, parse the micro version, \.[0-9]+
  if (input[0] != '.') return true;
  input = input.substr(1);
  if (parseInt(input, micro)) return true;

  // If we have characters left over, it's an error.
  if (!input.empty()) return true;

  *this = VersionTuple(major, minor, micro);
  return false;
}