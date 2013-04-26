
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

#include "Error.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class _readobj_error_category : public _do_message {
public:
  virtual const char* name() const;
  virtual std::string message(int ev) const;
  virtual error_condition default_error_condition(int ev) const;
};
} // namespace

const char *_readobj_error_category::name() const {
  return "llvm.readobj";
}

std::string _readobj_error_category::message(int ev) const {
  switch (ev) {
  case readobj_error::success: return "Success";
  case readobj_error::file_not_found:
    return "No such file.";
  case readobj_error::unsupported_file_format:
    return "The file was not recognized as a valid object file.";
  case readobj_error::unrecognized_file_format:
    return "Unrecognized file type.";
  case readobj_error::unsupported_obj_file_format:
    return "Unsupported object file format.";
  case readobj_error::unknown_symbol:
    return "Unknown symbol.";
  default:
    llvm_unreachable("An enumerator of readobj_error does not have a message "
                     "defined.");
  }
}

error_condition _readobj_error_category::default_error_condition(int ev) const {
  if (ev == readobj_error::success)
    return errc::success;
  return errc::invalid_argument;
}

namespace llvm {
const error_category &readobj_category() {
  static _readobj_error_category o;
  return o;
}
} // namespace llvm