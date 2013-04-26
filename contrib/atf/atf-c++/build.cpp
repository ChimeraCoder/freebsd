
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
#include "atf-c/build.h"
#include "atf-c/error.h"
#include "atf-c/utils.h"
}

#include "build.hpp"

#include "detail/exceptions.hpp"
#include "detail/process.hpp"

namespace impl = atf::build;
#define IMPL_NAME "atf::build"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

inline
atf::process::argv_array
cargv_to_argv(const atf_list_t* l)
{
    std::vector< const char* > aux;

    atf_list_citer_t iter;
    atf_list_for_each_c(iter, l)
        aux.push_back(static_cast< const char* >(atf_list_citer_data(iter)));

    return atf::process::argv_array(aux);
}

inline
atf::process::argv_array
cargv_to_argv_and_free(char** l)
{
    try {
        atf::process::argv_array argv((const char* const*)l);
        atf_utils_free_charpp(l);
        return argv;
    } catch (...) {
        atf_utils_free_charpp(l);
        throw;
    }
}

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

atf::process::argv_array
impl::c_o(const std::string& sfile, const std::string& ofile,
          const atf::process::argv_array& optargs)
{
    char** l;

    atf_error_t err = atf_build_c_o(sfile.c_str(), ofile.c_str(),
                                    optargs.exec_argv(), &l);
    if (atf_is_error(err))
        throw_atf_error(err);

    return cargv_to_argv_and_free(l);
}

atf::process::argv_array
impl::cpp(const std::string& sfile, const std::string& ofile,
          const atf::process::argv_array& optargs)
{
    char** l;

    atf_error_t err = atf_build_cpp(sfile.c_str(), ofile.c_str(),
                                    optargs.exec_argv(), &l);
    if (atf_is_error(err))
        throw_atf_error(err);

    return cargv_to_argv_and_free(l);
}

atf::process::argv_array
impl::cxx_o(const std::string& sfile, const std::string& ofile,
            const atf::process::argv_array& optargs)
{
    char** l;

    atf_error_t err = atf_build_cxx_o(sfile.c_str(), ofile.c_str(),
                                      optargs.exec_argv(), &l);
    if (atf_is_error(err))
        throw_atf_error(err);

    return cargv_to_argv_and_free(l);
}