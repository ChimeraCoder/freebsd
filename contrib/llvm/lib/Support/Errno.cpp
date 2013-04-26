
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

#include "llvm/Support/Errno.h"
#include "llvm/Config/config.h"     // Get autoconf configuration settings
#include "llvm/Support/raw_ostream.h"

#if HAVE_STRING_H
#include <string.h>

#if HAVE_ERRNO_H
#include <errno.h>
#endif

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

namespace llvm {
namespace sys {

#if HAVE_ERRNO_H
std::string StrError() {
  return StrError(errno);
}
#endif  // HAVE_ERRNO_H

std::string StrError(int errnum) {
  const int MaxErrStrLen = 2000;
  char buffer[MaxErrStrLen];
  buffer[0] = '\0';
  std::string str;
#ifdef HAVE_STRERROR_R
  // strerror_r is thread-safe.
  if (errnum)
# if defined(__GLIBC__) && defined(_GNU_SOURCE)
    // glibc defines its own incompatible version of strerror_r
    // which may not use the buffer supplied.
    str = strerror_r(errnum,buffer,MaxErrStrLen-1);
# else
    strerror_r(errnum,buffer,MaxErrStrLen-1);
    str = buffer;
# endif
#elif HAVE_DECL_STRERROR_S // "Windows Secure API"
    if (errnum) {
      strerror_s(buffer, MaxErrStrLen - 1, errnum);
      str = buffer;
    }
#elif defined(HAVE_STRERROR)
  // Copy the thread un-safe result of strerror into
  // the buffer as fast as possible to minimize impact
  // of collision of strerror in multiple threads.
  if (errnum)
    str = strerror(errnum);
#else
  // Strange that this system doesn't even have strerror
  // but, oh well, just use a generic message
  raw_string_ostream stream(str);
  stream << "Error #" << errnum;
  stream.flush();
#endif
  return str;
}

}  // namespace sys
}  // namespace llvm

#endif  // HAVE_STRING_H