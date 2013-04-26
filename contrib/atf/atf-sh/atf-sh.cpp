
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
#include <unistd.h>
}

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "atf-c++/config.hpp"

#include "atf-c++/detail/application.hpp"
#include "atf-c++/detail/fs.hpp"
#include "atf-c++/detail/sanity.hpp"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

namespace {

static
std::string
fix_plain_name(const char *filename)
{
    const atf::fs::path filepath(filename);
    if (filepath.branch_path().str() == ".")
        return std::string("./") + filename;
    else
        return std::string(filename);
}

static
std::string*
construct_script(const char* filename)
{
    const std::string libexecdir = atf::config::get("atf_libexecdir");
    const std::string pkgdatadir = atf::config::get("atf_pkgdatadir");
    const std::string shell = atf::config::get("atf_shell");

    std::string* command = new std::string();
    command->reserve(512);
    (*command) += ("Atf_Check='" + libexecdir + "/atf-check' ; " +
                   "Atf_Shell='" + shell + "' ; " +
                   ". " + pkgdatadir + "/libatf-sh.subr ; " +
                   ". " + fix_plain_name(filename) + " ; " +
                   "main \"${@}\"");
    return command;
}

static
const char**
construct_argv(const std::string& shell, const int interpreter_argc,
               const char* const* interpreter_argv)
{
    PRE(interpreter_argc >= 1);
    PRE(interpreter_argv[0] != NULL);

    const std::string* script = construct_script(interpreter_argv[0]);

    const int count = 4 + (interpreter_argc - 1) + 1;
    const char** argv = new const char*[count];
    argv[0] = shell.c_str();
    argv[1] = "-c";
    argv[2] = script->c_str();
    argv[3] = interpreter_argv[0];

    for (int i = 1; i < interpreter_argc; i++)
        argv[4 + i - 1] = interpreter_argv[i];

    argv[count - 1] = NULL;

    return argv;
}

} // anonymous namespace

// ------------------------------------------------------------------------
// The "atf_sh" class.
// ------------------------------------------------------------------------

class atf_sh : public atf::application::app {
    static const char* m_description;

public:
    atf_sh(void);

    int main(void);
};

const char* atf_sh::m_description =
    "atf-sh is a shell interpreter that extends the functionality of the "
    "system sh(1) with the atf-sh library.";

atf_sh::atf_sh(void) :
    app(m_description, "atf-sh(1)", "atf(7)")
{
}

int
atf_sh::main(void)
{
    if (m_argc < 1)
        throw atf::application::usage_error("No test program provided");

    const atf::fs::path script(m_argv[0]);
    if (!atf::fs::exists(script))
        throw std::runtime_error("The test program '" + script.str() + "' "
                                 "does not exist");

    const std::string shell = atf::config::get("atf_shell");
    const char** argv = construct_argv(shell, m_argc, m_argv);
    // Don't bother keeping track of the memory allocated by construct_argv:
    // we are going to exec or die immediately.

    const int ret = execv(shell.c_str(), const_cast< char** >(argv));
    INV(ret == -1);
    std::cerr << "Failed to execute " << shell << ": " << std::strerror(errno)
              << "\n";
    return EXIT_FAILURE;
}

int
main(int argc, char* const* argv)
{
    return atf_sh().run(argc, argv);
}