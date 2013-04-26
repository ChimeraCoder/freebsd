
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
#include <signal.h>
#include <unistd.h>
}

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "atf-c++/macros.hpp"

#include "atf-c++/detail/fs.hpp"

// ------------------------------------------------------------------------
// Helper tests for "t_config".
// ------------------------------------------------------------------------

ATF_TEST_CASE(config_unset);
ATF_TEST_CASE_HEAD(config_unset)
{
    set_md_var("descr", "Helper test case for the t_config test program");
}
ATF_TEST_CASE_BODY(config_unset)
{
    ATF_REQUIRE(!has_config_var("test"));
}

ATF_TEST_CASE(config_empty);
ATF_TEST_CASE_HEAD(config_empty)
{
    set_md_var("descr", "Helper test case for the t_config test program");
}
ATF_TEST_CASE_BODY(config_empty)
{
    ATF_REQUIRE_EQ(get_config_var("test"), "");
}

ATF_TEST_CASE(config_value);
ATF_TEST_CASE_HEAD(config_value)
{
    set_md_var("descr", "Helper test case for the t_config test program");
}
ATF_TEST_CASE_BODY(config_value)
{
    ATF_REQUIRE_EQ(get_config_var("test"), "foo");
}

ATF_TEST_CASE(config_multi_value);
ATF_TEST_CASE_HEAD(config_multi_value)
{
    set_md_var("descr", "Helper test case for the t_config test program");
}
ATF_TEST_CASE_BODY(config_multi_value)
{
    ATF_REQUIRE_EQ(get_config_var("test"), "foo bar");
}

// ------------------------------------------------------------------------
// Helper tests for "t_expect".
// ------------------------------------------------------------------------

ATF_TEST_CASE_WITHOUT_HEAD(expect_pass_and_pass);
ATF_TEST_CASE_BODY(expect_pass_and_pass)
{
    expect_pass();
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_pass_but_fail_requirement);
ATF_TEST_CASE_BODY(expect_pass_but_fail_requirement)
{
    expect_pass();
    fail("Some reason");
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_pass_but_fail_check);
ATF_TEST_CASE_BODY(expect_pass_but_fail_check)
{
    expect_pass();
    fail_nonfatal("Some reason");
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_fail_and_fail_requirement);
ATF_TEST_CASE_BODY(expect_fail_and_fail_requirement)
{
    expect_fail("Fail reason");
    fail("The failure");
    expect_pass();
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_fail_and_fail_check);
ATF_TEST_CASE_BODY(expect_fail_and_fail_check)
{
    expect_fail("Fail first");
    fail_nonfatal("abc");
    expect_pass();

    expect_fail("And fail again");
    fail_nonfatal("def");
    expect_pass();
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_fail_but_pass);
ATF_TEST_CASE_BODY(expect_fail_but_pass)
{
    expect_fail("Fail first");
    fail_nonfatal("abc");
    expect_pass();

    expect_fail("Will not fail");
    expect_pass();

    expect_fail("And fail again");
    fail_nonfatal("def");
    expect_pass();
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_exit_any_and_exit);
ATF_TEST_CASE_BODY(expect_exit_any_and_exit)
{
    expect_exit(-1, "Call will exit");
    std::exit(EXIT_SUCCESS);
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_exit_code_and_exit);
ATF_TEST_CASE_BODY(expect_exit_code_and_exit)
{
    expect_exit(123, "Call will exit");
    std::exit(123);
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_exit_but_pass);
ATF_TEST_CASE_BODY(expect_exit_but_pass)
{
    expect_exit(-1, "Call won't exit");
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_signal_any_and_signal);
ATF_TEST_CASE_BODY(expect_signal_any_and_signal)
{
    expect_signal(-1, "Call will signal");
    ::kill(getpid(), SIGKILL);
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_signal_no_and_signal);
ATF_TEST_CASE_BODY(expect_signal_no_and_signal)
{
    expect_signal(SIGHUP, "Call will signal");
    ::kill(getpid(), SIGHUP);
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_signal_but_pass);
ATF_TEST_CASE_BODY(expect_signal_but_pass)
{
    expect_signal(-1, "Call won't signal");
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_death_and_exit);
ATF_TEST_CASE_BODY(expect_death_and_exit)
{
    expect_death("Exit case");
    std::exit(123);
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_death_and_signal);
ATF_TEST_CASE_BODY(expect_death_and_signal)
{
    expect_death("Signal case");
    kill(getpid(), SIGKILL);
}

ATF_TEST_CASE_WITHOUT_HEAD(expect_death_but_pass);
ATF_TEST_CASE_BODY(expect_death_but_pass)
{
    expect_death("Call won't die");
}

ATF_TEST_CASE(expect_timeout_and_hang);
ATF_TEST_CASE_HEAD(expect_timeout_and_hang)
{
    set_md_var("timeout", "1");
}
ATF_TEST_CASE_BODY(expect_timeout_and_hang)
{
    expect_timeout("Will overrun");
    ::sleep(5);
}

ATF_TEST_CASE(expect_timeout_but_pass);
ATF_TEST_CASE_HEAD(expect_timeout_but_pass)
{
    set_md_var("timeout", "1");
}
ATF_TEST_CASE_BODY(expect_timeout_but_pass)
{
    expect_timeout("Will just exit");
}

// ------------------------------------------------------------------------
// Helper tests for "t_fork".
// ------------------------------------------------------------------------

ATF_TEST_CASE(fork_stop);
ATF_TEST_CASE_HEAD(fork_stop)
{
    set_md_var("descr", "Helper test case for the t_fork test program");
}
ATF_TEST_CASE_BODY(fork_stop)
{
    std::ofstream os(get_config_var("pidfile").c_str());
    os << ::getpid() << "\n";
    os.close();
    std::cout << "Wrote pid file\n";
    std::cout << "Waiting for done file\n";
    while (::access(get_config_var("donefile").c_str(), F_OK) != 0)
        ::usleep(10000);
    std::cout << "Exiting\n";
}

// ------------------------------------------------------------------------
// Helper tests for "t_meta_data".
// ------------------------------------------------------------------------

ATF_TEST_CASE(metadata_no_descr);
ATF_TEST_CASE_HEAD(metadata_no_descr)
{
}
ATF_TEST_CASE_BODY(metadata_no_descr)
{
}

ATF_TEST_CASE_WITHOUT_HEAD(metadata_no_head);
ATF_TEST_CASE_BODY(metadata_no_head)
{
}

// ------------------------------------------------------------------------
// Helper tests for "t_srcdir".
// ------------------------------------------------------------------------

ATF_TEST_CASE(srcdir_exists);
ATF_TEST_CASE_HEAD(srcdir_exists)
{
    set_md_var("descr", "Helper test case for the t_srcdir test program");
}
ATF_TEST_CASE_BODY(srcdir_exists)
{
    if (!atf::fs::exists(atf::fs::path(get_config_var("srcdir")) /
        "datafile"))
        ATF_FAIL("Cannot find datafile");
}

// ------------------------------------------------------------------------
// Helper tests for "t_result".
// ------------------------------------------------------------------------

ATF_TEST_CASE(result_pass);
ATF_TEST_CASE_HEAD(result_pass) { }
ATF_TEST_CASE_BODY(result_pass)
{
    std::cout << "msg\n";
}

ATF_TEST_CASE(result_fail);
ATF_TEST_CASE_HEAD(result_fail) { }
ATF_TEST_CASE_BODY(result_fail)
{
    std::cout << "msg\n";
    ATF_FAIL("Failure reason");
}

ATF_TEST_CASE(result_skip);
ATF_TEST_CASE_HEAD(result_skip) { }
ATF_TEST_CASE_BODY(result_skip)
{
    std::cout << "msg\n";
    ATF_SKIP("Skipped reason");
}

ATF_TEST_CASE(result_newlines_fail);
ATF_TEST_CASE_HEAD(result_newlines_fail)
{
    set_md_var("descr", "Helper test case for the t_result test program");
}
ATF_TEST_CASE_BODY(result_newlines_fail)
{
    ATF_FAIL("First line\nSecond line");
}

ATF_TEST_CASE(result_newlines_skip);
ATF_TEST_CASE_HEAD(result_newlines_skip)
{
    set_md_var("descr", "Helper test case for the t_result test program");
}
ATF_TEST_CASE_BODY(result_newlines_skip)
{
    ATF_SKIP("First line\nSecond line");
}

ATF_TEST_CASE(result_exception);
ATF_TEST_CASE_HEAD(result_exception) { }
ATF_TEST_CASE_BODY(result_exception)
{
    throw std::runtime_error("This is unhandled");
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add helper tests for t_config.
    ATF_ADD_TEST_CASE(tcs, config_unset);
    ATF_ADD_TEST_CASE(tcs, config_empty);
    ATF_ADD_TEST_CASE(tcs, config_value);
    ATF_ADD_TEST_CASE(tcs, config_multi_value);

    // Add helper tests for t_expect.
    ATF_ADD_TEST_CASE(tcs, expect_pass_and_pass);
    ATF_ADD_TEST_CASE(tcs, expect_pass_but_fail_requirement);
    ATF_ADD_TEST_CASE(tcs, expect_pass_but_fail_check);
    ATF_ADD_TEST_CASE(tcs, expect_fail_and_fail_requirement);
    ATF_ADD_TEST_CASE(tcs, expect_fail_and_fail_check);
    ATF_ADD_TEST_CASE(tcs, expect_fail_but_pass);
    ATF_ADD_TEST_CASE(tcs, expect_exit_any_and_exit);
    ATF_ADD_TEST_CASE(tcs, expect_exit_code_and_exit);
    ATF_ADD_TEST_CASE(tcs, expect_exit_but_pass);
    ATF_ADD_TEST_CASE(tcs, expect_signal_any_and_signal);
    ATF_ADD_TEST_CASE(tcs, expect_signal_no_and_signal);
    ATF_ADD_TEST_CASE(tcs, expect_signal_but_pass);
    ATF_ADD_TEST_CASE(tcs, expect_death_and_exit);
    ATF_ADD_TEST_CASE(tcs, expect_death_and_signal);
    ATF_ADD_TEST_CASE(tcs, expect_death_but_pass);
    ATF_ADD_TEST_CASE(tcs, expect_timeout_and_hang);
    ATF_ADD_TEST_CASE(tcs, expect_timeout_but_pass);

    // Add helper tests for t_fork.
    ATF_ADD_TEST_CASE(tcs, fork_stop);

    // Add helper tests for t_meta_data.
    ATF_ADD_TEST_CASE(tcs, metadata_no_descr);
    ATF_ADD_TEST_CASE(tcs, metadata_no_head);

    // Add helper tests for t_srcdir.
    ATF_ADD_TEST_CASE(tcs, srcdir_exists);

    // Add helper tests for t_result.
    ATF_ADD_TEST_CASE(tcs, result_pass);
    ATF_ADD_TEST_CASE(tcs, result_fail);
    ATF_ADD_TEST_CASE(tcs, result_skip);
    ATF_ADD_TEST_CASE(tcs, result_newlines_fail);
    ATF_ADD_TEST_CASE(tcs, result_newlines_skip);
    ATF_ADD_TEST_CASE(tcs, result_exception);
}