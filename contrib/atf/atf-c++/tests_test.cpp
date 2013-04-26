
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
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
}

#include <fstream>
#include <sstream>

#include "macros.hpp"

#include "detail/parser.hpp"
#include "detail/test_helpers.hpp"

// ------------------------------------------------------------------------
// Tests for the "atf_tp_writer" class.
// ------------------------------------------------------------------------

static
void
print_indented(const std::string& str)
{
    std::vector< std::string > ws = atf::text::split(str, "\n");
    for (std::vector< std::string >::const_iterator iter = ws.begin();
         iter != ws.end(); iter++)
        std::cout << ">>" << *iter << "<<\n";
}

// XXX Should this string handling and verbosity level be part of the
// ATF_REQUIRE_EQ macro?  It may be hard to predict sometimes that a
// string can have newlines in it, and so the error message generated
// at the moment will be bogus if there are some.
static
void
check_equal(const atf::tests::tc& tc, const std::string& str,
            const std::string& exp)
{
    if (str != exp) {
        std::cout << "String equality check failed.\n"
            "Adding >> and << to delimit the string boundaries below.\n";
        std::cout << "GOT:\n";
        print_indented(str);
        std::cout << "EXPECTED:\n";
        print_indented(exp);
        tc.fail("Constructed string differs from the expected one");
    }
}

ATF_TEST_CASE(atf_tp_writer);
ATF_TEST_CASE_HEAD(atf_tp_writer)
{
    set_md_var("descr", "Verifies the application/X-atf-tp writer");
}
ATF_TEST_CASE_BODY(atf_tp_writer)
{
    std::ostringstream expss;
    std::ostringstream ss;

#define RESET \
    expss.str(""); \
    ss.str("")

#define CHECK \
    check_equal(*this, ss.str(), expss.str())

    {
        RESET;

        atf::tests::detail::atf_tp_writer w(ss);
        expss << "Content-Type: application/X-atf-tp; version=\"1\"\n\n";
        CHECK;
    }

    {
        RESET;

        atf::tests::detail::atf_tp_writer w(ss);
        expss << "Content-Type: application/X-atf-tp; version=\"1\"\n\n";
        CHECK;

        w.start_tc("test1");
        expss << "ident: test1\n";
        CHECK;

        w.end_tc();
        CHECK;
    }

    {
        RESET;

        atf::tests::detail::atf_tp_writer w(ss);
        expss << "Content-Type: application/X-atf-tp; version=\"1\"\n\n";
        CHECK;

        w.start_tc("test1");
        expss << "ident: test1\n";
        CHECK;

        w.end_tc();
        CHECK;

        w.start_tc("test2");
        expss << "\nident: test2\n";
        CHECK;

        w.end_tc();
        CHECK;
    }

    {
        RESET;

        atf::tests::detail::atf_tp_writer w(ss);
        expss << "Content-Type: application/X-atf-tp; version=\"1\"\n\n";
        CHECK;

        w.start_tc("test1");
        expss << "ident: test1\n";
        CHECK;

        w.tc_meta_data("descr", "the description");
        expss << "descr: the description\n";
        CHECK;

        w.end_tc();
        CHECK;

        w.start_tc("test2");
        expss << "\nident: test2\n";
        CHECK;

        w.tc_meta_data("descr", "second test case");
        expss << "descr: second test case\n";
        CHECK;

        w.tc_meta_data("require.progs", "/bin/cp");
        expss << "require.progs: /bin/cp\n";
        CHECK;

        w.tc_meta_data("X-custom", "foo bar baz");
        expss << "X-custom: foo bar baz\n";
        CHECK;

        w.end_tc();
        CHECK;
    }

#undef CHECK
#undef RESET
}

// ------------------------------------------------------------------------
// Tests cases for the header file.
// ------------------------------------------------------------------------

HEADER_TC(include, "atf-c++/tests.hpp");

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add tests for the "atf_tp_writer" class.
    ATF_ADD_TEST_CASE(tcs, atf_tp_writer);

    // Add the test cases for the header file.
    ATF_ADD_TEST_CASE(tcs, include);
}