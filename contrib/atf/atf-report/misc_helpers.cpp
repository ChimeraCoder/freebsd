
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

#include <iostream>

#include "atf-c++/macros.hpp"

// ------------------------------------------------------------------------
// Helper tests for "t_integration".
// ------------------------------------------------------------------------

ATF_TEST_CASE(diff);
ATF_TEST_CASE_HEAD(diff)
{
    set_md_var("descr", "Helper test case for the t_integration program");
}
ATF_TEST_CASE_BODY(diff)
{
    std::cout << "--- a	2007-11-04 14:00:41.000000000 +0100\n";
    std::cout << "+++ b	2007-11-04 14:00:48.000000000 +0100\n";
    std::cout << "@@ -1,7 +1,7 @@\n";
    std::cout << " This test is meant to simulate a diff.\n";
    std::cout << " Blank space at beginning of context lines must be "
                 "preserved.\n";
    std::cout << " \n";
    std::cout << "-First original line.\n";
    std::cout << "-Second original line.\n";
    std::cout << "+First modified line.\n";
    std::cout << "+Second modified line.\n";
    std::cout << " \n";
    std::cout << " EOF\n";
}

// ------------------------------------------------------------------------
// Main.
// ------------------------------------------------------------------------

ATF_INIT_TEST_CASES(tcs)
{
    // Add helper tests for t_integration.
    ATF_ADD_TEST_CASE(tcs, diff);
}