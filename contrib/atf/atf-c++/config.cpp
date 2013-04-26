
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

#include <map>

extern "C" {
#include "atf-c/config.h"
}

#include "config.hpp"

#include "detail/env.hpp"
#include "detail/sanity.hpp"

static std::map< std::string, std::string > m_variables;

//
// Adds all predefined standard build-time variables to the m_variables
// map, considering the values a user may have provided in the environment.
//
// Can only be called once during the program's lifetime.
//
static
void
init_variables(void)
{
    PRE(m_variables.empty());

    m_variables["atf_arch"] = atf_config_get("atf_arch");
    m_variables["atf_build_cc"] = atf_config_get("atf_build_cc");
    m_variables["atf_build_cflags"] = atf_config_get("atf_build_cflags");
    m_variables["atf_build_cpp"] = atf_config_get("atf_build_cpp");
    m_variables["atf_build_cppflags"] = atf_config_get("atf_build_cppflags");
    m_variables["atf_build_cxx"] = atf_config_get("atf_build_cxx");
    m_variables["atf_build_cxxflags"] = atf_config_get("atf_build_cxxflags");
    m_variables["atf_confdir"] = atf_config_get("atf_confdir");
    m_variables["atf_includedir"] = atf_config_get("atf_includedir");
    m_variables["atf_libdir"] = atf_config_get("atf_libdir");
    m_variables["atf_libexecdir"] = atf_config_get("atf_libexecdir");
    m_variables["atf_machine"] = atf_config_get("atf_machine");
    m_variables["atf_pkgdatadir"] = atf_config_get("atf_pkgdatadir");
    m_variables["atf_shell"] = atf_config_get("atf_shell");
    m_variables["atf_workdir"] = atf_config_get("atf_workdir");

    POST(!m_variables.empty());
}

const std::string&
atf::config::get(const std::string& varname)
{
    if (m_variables.empty())
        init_variables();

    PRE(has(varname));
    return m_variables[varname];
}

const std::map< std::string, std::string >&
atf::config::get_all(void)
{
    if (m_variables.empty())
        init_variables();

    return m_variables;
}

bool
atf::config::has(const std::string& varname)
{
    if (m_variables.empty())
        init_variables();

    return m_variables.find(varname) != m_variables.end();
}

extern "C" {
void __atf_config_reinit(void);
}

namespace atf {
namespace config {
//
// Auxiliary function for the t_config test program so that it can
// revert the configuration's global status to an empty state and
// do new tests from there on.
//
// Ideally this shouldn't be part of the production library... but
// this is so small that it does not matter.
//
void
__reinit(void)
{
    __atf_config_reinit();
    m_variables.clear();
}
} // namespace config
} // namespace atf