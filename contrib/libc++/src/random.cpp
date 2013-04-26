
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

#include "random"
#include "system_error"

#ifdef __sun__
#define rename solaris_headers_are_broken
#endif
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

_LIBCPP_BEGIN_NAMESPACE_STD

random_device::random_device(const string& __token)
    : __f_(open(__token.c_str(), O_RDONLY))
{
    if (__f_ <= 0)
        __throw_system_error(errno, ("random_device failed to open " + __token).c_str());
}

random_device::~random_device()
{
    close(__f_);
}

unsigned
random_device::operator()()
{
    unsigned r;
    read(__f_, &r, sizeof(r));
    return r;
}

double
random_device::entropy() const _NOEXCEPT
{
    return 0;
}

_LIBCPP_END_NAMESPACE_STD