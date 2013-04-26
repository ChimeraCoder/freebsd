
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
#include "../../atf-c/error.h"

#include "../../atf-c/detail/env.h"
}

#include "env.hpp"
#include "exceptions.hpp"
#include "sanity.hpp"

namespace impl = atf::env;
#define IMPL_NAME "atf::env"

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

std::string
impl::get(const std::string& name)
{
    return atf_env_get(name.c_str());
}

bool
impl::has(const std::string& name)
{
    return atf_env_has(name.c_str());
}

void
impl::set(const std::string& name, const std::string& val)
{
    atf_error_t err = atf_env_set(name.c_str(), val.c_str());
    if (atf_is_error(err))
        throw_atf_error(err);
}

void
impl::unset(const std::string& name)
{
    atf_error_t err = atf_env_unset(name.c_str());
    if (atf_is_error(err))
        throw_atf_error(err);
}