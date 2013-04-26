
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

#include "../macros.hpp"

#include "env.hpp"

// ------------------------------------------------------------------------
// Test cases for the free functions.
// ------------------------------------------------------------------------

ATF_TEST_CASE(has_get);
ATF_TEST_CASE_HEAD(has_get)
{
    set_md_var("descr", "Tests the has and get functions");
}
ATF_TEST_CASE_BODY(has_get)
{
    ATF_REQUIRE(atf::env::has("PATH"));
    ATF_REQUIRE(!atf::env::get("PATH").empty());

    ATF_REQUIRE(!atf::env::has("_UNDEFINED_VARIABLE_"));
}

ATF_TEST_CASE(set);
ATF_TEST_CASE_HEAD(set)
{
    set_md_var("descr", "Tests the set function");
}
ATF_TEST_CASE_BODY(set)
{
    ATF_REQUIRE(atf::env::has("PATH"));
    const std::string& oldval = atf::env::get("PATH");
    atf::env::set("PATH", "foo-bar");
    ATF_REQUIRE(atf::env::get("PATH") != oldval);
    ATF_REQUIRE_EQ(atf::env::get("PATH"), "foo-bar");

    ATF_REQUIRE(!atf::env::has("_UNDEFINED_VARIABLE_"));
    atf::env::set("_UNDEFINED_VARIABLE_", "foo2-bar2");
    ATF_REQUIRE_EQ(atf::env::get("_UNDEFINED_VARIABLE_"), "foo2-bar2");
}

ATF_TEST_CASE(unset);
ATF_TEST_CASE_HEAD(unset)
{
    set_md_var("descr", "Tests the unset function");
}
ATF_TEST_CASE_BODY(unset)
{
    ATF_REQUIRE(atf::env::has("PATH"));
    atf::env::unset("PATH");
    ATF_REQUIRE(!atf::env::has("PATH"));
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add the test cases for the free functions.
    ATF_ADD_TEST_CASE(tcs, has_get);
    ATF_ADD_TEST_CASE(tcs, set);
    ATF_ADD_TEST_CASE(tcs, unset);
}