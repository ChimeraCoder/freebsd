
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

#include <cstring>

extern "C" {
#include "atf-c/build.h"
#include "atf-c/error.h"
}

#include "check.hpp"

#include "detail/exceptions.hpp"
#include "detail/process.hpp"
#include "detail/sanity.hpp"

namespace impl = atf::check;
#define IMPL_NAME "atf::check"

// ------------------------------------------------------------------------
// The "check_result" class.
// ------------------------------------------------------------------------

impl::check_result::check_result(const atf_check_result_t* result)
{
    std::memcpy(&m_result, result, sizeof(m_result));
}

impl::check_result::~check_result(void)
{
    atf_check_result_fini(&m_result);
}

bool
impl::check_result::exited(void)
    const
{
    return atf_check_result_exited(&m_result);
}

int
impl::check_result::exitcode(void)
    const
{
    PRE(exited());
    return atf_check_result_exitcode(&m_result);
}

bool
impl::check_result::signaled(void)
    const
{
    return atf_check_result_signaled(&m_result);
}

int
impl::check_result::termsig(void)
    const
{
    PRE(signaled());
    return atf_check_result_termsig(&m_result);
}

const std::string
impl::check_result::stdout_path(void) const
{
    return atf_check_result_stdout(&m_result);
}

const std::string
impl::check_result::stderr_path(void) const
{
    return atf_check_result_stderr(&m_result);
}

// ------------------------------------------------------------------------
// Free functions.
// ------------------------------------------------------------------------

bool
impl::build_c_o(const std::string& sfile, const std::string& ofile,
                const atf::process::argv_array& optargs)
{
    bool success;

    atf_error_t err = atf_check_build_c_o(sfile.c_str(), ofile.c_str(),
                                          optargs.exec_argv(), &success);
    if (atf_is_error(err))
        throw_atf_error(err);

    return success;
}

bool
impl::build_cpp(const std::string& sfile, const std::string& ofile,
                const atf::process::argv_array& optargs)
{
    bool success;

    atf_error_t err = atf_check_build_cpp(sfile.c_str(), ofile.c_str(),
                                          optargs.exec_argv(), &success);
    if (atf_is_error(err))
        throw_atf_error(err);

    return success;
}

bool
impl::build_cxx_o(const std::string& sfile, const std::string& ofile,
                  const atf::process::argv_array& optargs)
{
    bool success;

    atf_error_t err = atf_check_build_cxx_o(sfile.c_str(), ofile.c_str(),
                                            optargs.exec_argv(), &success);
    if (atf_is_error(err))
        throw_atf_error(err);

    return success;
}

std::auto_ptr< impl::check_result >
impl::exec(const atf::process::argv_array& argva)
{
    atf_check_result_t result;

    atf_error_t err = atf_check_exec_array(argva.exec_argv(), &result);
    if (atf_is_error(err))
        throw_atf_error(err);

    return std::auto_ptr< impl::check_result >(new impl::check_result(&result));
}