
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

#include <atf-c/macros.h>

ATF_TC(this_is_used);
ATF_TC_HEAD(this_is_used, tc)
{
    atf_tc_set_md_var(tc, "descr", "A test case that is not referenced");
}
ATF_TC_BODY(this_is_used, tc)
{
}

ATF_TC(this_is_unused);
ATF_TC_HEAD(this_is_unused, tc)
{
    atf_tc_set_md_var(tc, "descr", "A test case that is referenced");
}
ATF_TC_BODY(this_is_unused, tc)
{
}

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, this_is_used);
    /* ATF_TP_ADD_TC(tp, this_is_unused); */

    return atf_no_error();
}