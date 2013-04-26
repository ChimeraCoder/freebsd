
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

#include <stdexcept>

#include <atf-c++/macros.hpp>

void
atf_check_errno_semicolons(void)
{
    // Check that ATF_CHECK_ERRNO does not contain a semicolon that would
    // cause an empty-statement that confuses some compilers.
    ATF_CHECK_ERRNO(1, 1 == 1);
    ATF_CHECK_ERRNO(2, 2 == 2);
}

void
atf_require_inside_if(void)
{
    // Make sure that ATF_REQUIRE can be used inside an if statement that
    // does not have braces.  Earlier versions of it generated an error
    // if there was an else clause because they confused the compiler
    // by defining an unprotected nested if.
    if (true)
        ATF_REQUIRE(true);
    else
        ATF_REQUIRE(true);
}

void
atf_require_eq_inside_if(void)
{
    // Make sure that ATF_REQUIRE_EQ can be used inside an if statement
    // that does not have braces.  Earlier versions of it generated an
    // error if there was an else clause because they confused the
    // compiler by defining an unprotected nested if.
    if (true)
        ATF_REQUIRE_EQ(true, true);
    else
        ATF_REQUIRE_EQ(true, true);
}

void
atf_require_throw_runtime_error(void)
{
    // Check that we can pass std::runtime_error to ATF_REQUIRE_THROW.
    // Earlier versions generated a warning because the macro's code also
    // attempted to capture this exception, and thus we had a duplicate
    // catch clause.
    ATF_REQUIRE_THROW(std::runtime_error, (void)0);
}

void
atf_require_throw_inside_if(void)
{
    // Make sure that ATF_REQUIRE_THROW can be used inside an if statement
    // that does not have braces.  Earlier versions of it generated an
    // error because a trailing ; after a catch block was not allowed.
    if (true)
        ATF_REQUIRE_THROW(std::runtime_error, (void)0);
    else
        ATF_REQUIRE_THROW(std::runtime_error, (void)1);
}

void
atf_require_errno_semicolons(void)
{
    // Check that ATF_REQUIRE_ERRNO does not contain a semicolon that would
    // cause an empty-statement that confuses some compilers.
    ATF_REQUIRE_ERRNO(1, 1 == 1);
    ATF_REQUIRE_ERRNO(2, 2 == 2);
}

// Test case names should not be expanded during instatiation so that they
// can have the exact same name as macros.
#define TEST_MACRO_1 invalid + name
#define TEST_MACRO_2 invalid + name
#define TEST_MACRO_3 invalid + name
ATF_TEST_CASE(TEST_MACRO_1);
ATF_TEST_CASE_HEAD(TEST_MACRO_1) { }
ATF_TEST_CASE_BODY(TEST_MACRO_1) { }
void instantiate_1(void) {
    ATF_TEST_CASE_USE(TEST_MACRO_1);
    atf::tests::tc* the_test = new ATF_TEST_CASE_NAME(TEST_MACRO_1)();
    delete the_test;
}
ATF_TEST_CASE_WITH_CLEANUP(TEST_MACRO_2);
ATF_TEST_CASE_HEAD(TEST_MACRO_2) { }
ATF_TEST_CASE_BODY(TEST_MACRO_2) { }
ATF_TEST_CASE_CLEANUP(TEST_MACRO_2) { }
void instatiate_2(void) {
    ATF_TEST_CASE_USE(TEST_MACRO_2);
    atf::tests::tc* the_test = new ATF_TEST_CASE_NAME(TEST_MACRO_2)();
    delete the_test;
}
ATF_TEST_CASE_WITH_CLEANUP(TEST_MACRO_3);
ATF_TEST_CASE_HEAD(TEST_MACRO_3) { }
ATF_TEST_CASE_BODY(TEST_MACRO_3) { }
ATF_TEST_CASE_CLEANUP(TEST_MACRO_3) { }
void instatiate_3(void) {
    ATF_TEST_CASE_USE(TEST_MACRO_3);
    atf::tests::tc* the_test = new ATF_TEST_CASE_NAME(TEST_MACRO_3)();
    delete the_test;
}