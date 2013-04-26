
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

#include <atf-c++/macros.hpp>

ATF_TEST_CASE(this_is_used);
ATF_TEST_CASE_HEAD(this_is_used)
{
}
ATF_TEST_CASE_BODY(this_is_used)
{
}

ATF_TEST_CASE(this_is_unused);
ATF_TEST_CASE_HEAD(this_is_unused)
{
}
ATF_TEST_CASE_BODY(this_is_unused)
{
}

ATF_INIT_TEST_CASES(tcs)
{
    ATF_ADD_TEST_CASE(tcs, this_is_used);
    //ATF_ADD_TEST_CASE(tcs, this_is_unused);
}