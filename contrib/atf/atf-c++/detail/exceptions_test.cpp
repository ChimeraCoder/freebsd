
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
}

#include <cstdio>
#include <new>

#include "../macros.hpp"

#include "exceptions.hpp"
#include "sanity.hpp"

// ------------------------------------------------------------------------
// The "test" error.
// ------------------------------------------------------------------------

extern "C" {

struct test_error_data {
    const char* m_msg;
};
typedef struct test_error_data test_error_data_t;

static
void
test_format(const atf_error_t err, char *buf, size_t buflen)
{
    const test_error_data_t* data;

    PRE(atf_error_is(err, "test"));

    data = static_cast< const test_error_data_t * >(atf_error_data(err));
    snprintf(buf, buflen, "Message: %s", data->m_msg);
}

static
atf_error_t
test_error(const char* msg)
{
    atf_error_t err;
    test_error_data_t data;

    data.m_msg = msg;

    err = atf_error_new("test", &data, sizeof(data), test_format);

    return err;
}

} // extern

// ------------------------------------------------------------------------
// Tests cases for the free functions.
// ------------------------------------------------------------------------

ATF_TEST_CASE(throw_atf_error_libc);
ATF_TEST_CASE_HEAD(throw_atf_error_libc)
{
    set_md_var("descr", "Tests the throw_atf_error function when raising "
               "a libc error");
}
ATF_TEST_CASE_BODY(throw_atf_error_libc)
{
    try {
        atf::throw_atf_error(atf_libc_error(1, "System error 1"));
    } catch (const atf::system_error& e) {
        ATF_REQUIRE(e.code() == 1);
        ATF_REQUIRE(std::string(e.what()).find("System error 1") !=
                  std::string::npos);
    } catch (const std::exception& e) {
        ATF_FAIL(std::string("Got unexpected exception: ") + e.what());
    }
}

ATF_TEST_CASE(throw_atf_error_no_memory);
ATF_TEST_CASE_HEAD(throw_atf_error_no_memory)
{
    set_md_var("descr", "Tests the throw_atf_error function when raising "
               "a no_memory error");
}
ATF_TEST_CASE_BODY(throw_atf_error_no_memory)
{
    try {
        atf::throw_atf_error(atf_no_memory_error());
    } catch (const std::bad_alloc&) {
    } catch (const std::exception& e) {
        ATF_FAIL(std::string("Got unexpected exception: ") + e.what());
    }
}

ATF_TEST_CASE(throw_atf_error_unknown);
ATF_TEST_CASE_HEAD(throw_atf_error_unknown)
{
    set_md_var("descr", "Tests the throw_atf_error function when raising "
               "an unknown error");
}
ATF_TEST_CASE_BODY(throw_atf_error_unknown)
{
    try {
        atf::throw_atf_error(test_error("The message"));
    } catch (const std::runtime_error& e) {
        const std::string msg = e.what();
        ATF_REQUIRE(msg.find("The message") != std::string::npos);
    } catch (const std::exception& e) {
        ATF_FAIL(std::string("Got unexpected exception: ") + e.what());
    }
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the test cases for the free functions.
    ATF_ADD_TEST_CASE(tcs, throw_atf_error_libc);
    ATF_ADD_TEST_CASE(tcs, throw_atf_error_no_memory);
    ATF_ADD_TEST_CASE(tcs, throw_atf_error_unknown);
}