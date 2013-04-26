
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

extern "C" {
#include <sys/types.h>

#include <pwd.h>
#include <unistd.h>

#include "../atf-c/detail/user.h"
}

#include <stdexcept>
#include <string>

#include "../atf-c++/detail/sanity.hpp"

#include "user.hpp"

namespace impl = atf::atf_run;
#define IMPL_NAME "atf::atf_run"

uid_t
impl::euid(void)
{
    return atf_user_euid();
}

void
impl::drop_privileges(const std::pair< int, int > ids)
{
    if (::setgid(ids.second) == -1)
        throw std::runtime_error("Failed to drop group privileges");
    if (::setuid(ids.first) == -1)
        throw std::runtime_error("Failed to drop user privileges");
}

std::pair< int, int >
impl::get_user_ids(const std::string& user)
{
    const struct passwd* pw = ::getpwnam(user.c_str());
    if (pw == NULL)
        throw std::runtime_error("Failed to get information for user " + user);
    return std::make_pair(pw->pw_uid, pw->pw_gid);
}

bool
impl::is_member_of_group(gid_t gid)
{
    return atf_user_is_member_of_group(gid);
}

bool
impl::is_root(void)
{
    return atf_user_is_root();
}

bool
impl::is_unprivileged(void)
{
    return atf_user_is_unprivileged();
}