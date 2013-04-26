
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

#include "llvm/Object/Error.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;
using namespace object;

namespace {
class _object_error_category : public _do_message {
public:
  virtual const char* name() const;
  virtual std::string message(int ev) const;
  virtual error_condition default_error_condition(int ev) const;
};
}

const char *_object_error_category::name() const {
  return "llvm.object";
}

std::string _object_error_category::message(int ev) const {
  switch (ev) {
  case object_error::success: return "Success";
  case object_error::invalid_file_type:
    return "The file was not recognized as a valid object file";
  case object_error::parse_failed:
    return "Invalid data was encountered while parsing the file";
  case object_error::unexpected_eof:
    return "The end of the file was unexpectedly encountered";
  default:
    llvm_unreachable("An enumerator of object_error does not have a message "
                     "defined.");
  }
}

error_condition _object_error_category::default_error_condition(int ev) const {
  if (ev == object_error::success)
    return errc::success;
  return errc::invalid_argument;
}

const error_category &object::object_category() {
  static _object_error_category o;
  return o;
}