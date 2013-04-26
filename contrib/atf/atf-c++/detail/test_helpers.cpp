
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
#include <regex.h>
}

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../check.hpp"
#include "../config.hpp"
#include "../macros.hpp"

#include "fs.hpp"
#include "process.hpp"
#include "test_helpers.hpp"

void
build_check_cxx_o_aux(const atf::fs::path& sfile, const char* failmsg,
                      const bool expect_pass)
{
    std::vector< std::string > optargs;
    optargs.push_back("-I" + atf::config::get("atf_includedir"));
    optargs.push_back("-Wall");
    optargs.push_back("-Werror");

    const bool result = atf::check::build_cxx_o(
        sfile.str(), "test.o", atf::process::argv_array(optargs));
    if ((expect_pass && !result) || (!expect_pass && result))
        ATF_FAIL(failmsg);
}

void
build_check_cxx_o(const atf::tests::tc& tc, const char* sfile,
                  const char* failmsg, const bool expect_pass)
{
    const atf::fs::path sfilepath =
        atf::fs::path(tc.get_config_var("srcdir")) / sfile;
    build_check_cxx_o_aux(sfilepath, failmsg, expect_pass);
}

void
header_check(const char *hdrname)
{
    std::ofstream srcfile("test.c");
    ATF_REQUIRE(srcfile);
    srcfile << "#include <" << hdrname << ">\n";
    srcfile.close();

    const std::string failmsg = std::string("Header check failed; ") +
        hdrname + " is not self-contained";
    build_check_cxx_o_aux(atf::fs::path("test.c"), failmsg.c_str(), true);
}

atf::fs::path
get_process_helpers_path(const atf::tests::tc& tc)
{
    return atf::fs::path(tc.get_config_var("srcdir")) /
           ".." / "atf-c" / "detail" / "process_helpers";
}

bool
grep_file(const char* name, const char* regex)
{
    std::ifstream is(name);
    ATF_REQUIRE(is);

    bool found = false;

    std::string line;
    std::getline(is, line);
    while (!found && is.good()) {
        if (grep_string(line, regex))
            found = true;
        else
            std::getline(is, line);
    }

    return found;
}

bool
grep_string(const std::string& str, const char* regex)
{
    int res;
    regex_t preg;

    std::cout << "Looking for '" << regex << "' in '" << str << "'\n";
    ATF_REQUIRE(::regcomp(&preg, regex, REG_EXTENDED) == 0);

    res = ::regexec(&preg, str.c_str(), 0, NULL, 0);
    ATF_REQUIRE(res == 0 || res == REG_NOMATCH);

    ::regfree(&preg);

    return res == 0;
}

void
test_helpers_detail::check_equal(const char* expected[],
                                 const string_vector& actual)
{
    const char** expected_iter = expected;
    string_vector::const_iterator actual_iter = actual.begin();

    bool equals = true;
    while (equals && *expected_iter != NULL && actual_iter != actual.end()) {
        if (*expected_iter != *actual_iter) {
            equals = false;
        } else {
            expected_iter++;
            actual_iter++;
        }
    }
    if (equals && ((*expected_iter == NULL && actual_iter != actual.end()) ||
                   (*expected_iter != NULL && actual_iter == actual.end())))
        equals = false;

    if (!equals) {
        std::cerr << "EXPECTED:\n";
        for (expected_iter = expected; *expected_iter != NULL; expected_iter++)
            std::cerr << *expected_iter << "\n";

        std::cerr << "ACTUAL:\n";
        for (actual_iter = actual.begin(); actual_iter != actual.end();
             actual_iter++)
            std::cerr << *actual_iter << "\n";

        ATF_FAIL("Expected results differ to actual values");
    }
}