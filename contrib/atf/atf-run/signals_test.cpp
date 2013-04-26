
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
#include <signal.h>
#include <unistd.h>
}

#include <cerrno>
#include <cstdlib>
#include <iostream>

#include "atf-c/defs.h"

#include "atf-c++/macros.hpp"

#include "atf-c++/detail/exceptions.hpp"
#include "atf-c++/detail/process.hpp"

#include "signals.hpp"

// ------------------------------------------------------------------------
// Auxiliary functions.
// ------------------------------------------------------------------------

namespace sigusr1 {
    static bool happened = false;

    static
    void
    handler(int signo ATF_DEFS_ATTRIBUTE_UNUSED)
    {
        happened = true;
    }

    static
    void
    program(void)
    {
        struct sigaction sa;
        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (::sigaction(SIGUSR1, &sa, NULL) == -1)
            throw atf::system_error("sigusr1::program",
                                    "sigaction(2) failed", errno);
    }
} // namespace sigusr1

namespace sigusr1_2 {
    static bool happened = false;

    static
    void
    handler(int signo ATF_DEFS_ATTRIBUTE_UNUSED)
    {
        happened = true;
    }
} // namespace sigusr1_2

// ------------------------------------------------------------------------
// Tests for the "signal_holder" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(signal_holder_preserve);
ATF_TEST_CASE_HEAD(signal_holder_preserve)
{
    set_md_var("descr", "Tests that signal_holder preserves the original "
               "signal handler and restores it upon destruction");
}
ATF_TEST_CASE_BODY(signal_holder_preserve)
{
    using atf::atf_run::signal_holder;

    sigusr1::program();

    sigusr1::happened = false;
    ::kill(::getpid(), SIGUSR1);
    ATF_REQUIRE(sigusr1::happened);

    {
        signal_holder hld(SIGUSR1);
        ::kill(::getpid(), SIGUSR1);
    }

    sigusr1::happened = false;
    ::kill(::getpid(), SIGUSR1);
    ATF_REQUIRE(sigusr1::happened);
}

ATF_TEST_CASE(signal_holder_destructor);
ATF_TEST_CASE_HEAD(signal_holder_destructor)
{
    set_md_var("descr", "Tests that signal_holder processes a pending "
               "signal upon destruction");
}
ATF_TEST_CASE_BODY(signal_holder_destructor)
{
    using atf::atf_run::signal_holder;

    sigusr1::program();

    sigusr1::happened = false;
    ::kill(::getpid(), SIGUSR1);
    ATF_REQUIRE(sigusr1::happened);

    {
        signal_holder hld(SIGUSR1);

        sigusr1::happened = false;
        ::kill(::getpid(), SIGUSR1);
        ATF_REQUIRE(!sigusr1::happened);
    }
    ATF_REQUIRE(sigusr1::happened);
}

ATF_TEST_CASE(signal_holder_process);
ATF_TEST_CASE_HEAD(signal_holder_process)
{
    set_md_var("descr", "Tests that signal_holder's process method works "
               "to process a delayed signal explicitly");
}
ATF_TEST_CASE_BODY(signal_holder_process)
{
    using atf::atf_run::signal_holder;

    sigusr1::program();

    sigusr1::happened = false;
    ::kill(::getpid(), SIGUSR1);
    ATF_REQUIRE(sigusr1::happened);

    {
        signal_holder hld(SIGUSR1);

        sigusr1::happened = false;
        ::kill(::getpid(), SIGUSR1);
        ATF_REQUIRE(!sigusr1::happened);

        hld.process();
        ATF_REQUIRE(sigusr1::happened);

        sigusr1::happened = false;
    }
    ATF_REQUIRE(!sigusr1::happened);
}

// ------------------------------------------------------------------------
// Tests for the "signal_programmer" class.
// ------------------------------------------------------------------------

ATF_TEST_CASE(signal_programmer_program);
ATF_TEST_CASE_HEAD(signal_programmer_program)
{
    set_md_var("descr", "Tests that signal_programmer correctly installs a "
               "handler");
}
ATF_TEST_CASE_BODY(signal_programmer_program)
{
    using atf::atf_run::signal_programmer;

    signal_programmer sp(SIGUSR1, sigusr1_2::handler);

    sigusr1_2::happened = false;
    ::kill(::getpid(), SIGUSR1);
    ATF_REQUIRE(sigusr1_2::happened);
}

ATF_TEST_CASE(signal_programmer_preserve);
ATF_TEST_CASE_HEAD(signal_programmer_preserve)
{
    set_md_var("descr", "Tests that signal_programmer uninstalls the "
               "handler during destruction");
}
ATF_TEST_CASE_BODY(signal_programmer_preserve)
{
    using atf::atf_run::signal_programmer;

    sigusr1::program();
    sigusr1::happened = false;

    {
        signal_programmer sp(SIGUSR1, sigusr1_2::handler);

        sigusr1_2::happened = false;
        ::kill(::getpid(), SIGUSR1);
        ATF_REQUIRE(sigusr1_2::happened);
    }

    ATF_REQUIRE(!sigusr1::happened);
    ::kill(::getpid(), SIGUSR1);
    ATF_REQUIRE(sigusr1::happened);
}

// ------------------------------------------------------------------------
// Tests cases for the free functions.
// ------------------------------------------------------------------------

static
void
reset_child(void *v ATF_DEFS_ATTRIBUTE_UNUSED)
{
    sigusr1::program();

    sigusr1::happened = false;
    atf::atf_run::reset(SIGUSR1);
    kill(::getpid(), SIGUSR1);

    if (sigusr1::happened) {
        std::cerr << "Signal was not resetted correctly\n";
        std::abort();
    } else {
        std::exit(EXIT_SUCCESS);
    }
}

ATF_TEST_CASE(reset);
ATF_TEST_CASE_HEAD(reset)
{
    set_md_var("descr", "Tests the reset function");
}
ATF_TEST_CASE_BODY(reset)
{
    atf::process::child c =
        atf::process::fork(reset_child, atf::process::stream_inherit(),
                           atf::process::stream_inherit(), NULL);

    const atf::process::status s = c.wait();
    ATF_REQUIRE(s.exited() || s.signaled());
    ATF_REQUIRE(!s.signaled() || s.termsig() == SIGUSR1);
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the tests for the "signal_holder" class.
    ATF_ADD_TEST_CASE(tcs, signal_holder_preserve);
    ATF_ADD_TEST_CASE(tcs, signal_holder_destructor);
    ATF_ADD_TEST_CASE(tcs, signal_holder_process);

    // Add the tests for the "signal_programmer" class.
    ATF_ADD_TEST_CASE(tcs, signal_programmer_program);
    ATF_ADD_TEST_CASE(tcs, signal_programmer_preserve);

    // Add the test cases for the free functions.
    ATF_ADD_TEST_CASE(tcs, reset);
}