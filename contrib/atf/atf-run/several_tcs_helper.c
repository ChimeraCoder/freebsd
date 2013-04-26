
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

#include <atf-c.h>

ATF_TC(first);
ATF_TC_HEAD(first, tc)
{
    atf_tc_set_md_var(tc, "descr", "Description 1");
}
ATF_TC_BODY(first, tc)
{
}

ATF_TC_WITH_CLEANUP(second);
ATF_TC_HEAD(second, tc)
{
    atf_tc_set_md_var(tc, "descr", "Description 2");
    atf_tc_set_md_var(tc, "timeout", "500");
    atf_tc_set_md_var(tc, "X-property", "Custom property");
}
ATF_TC_BODY(second, tc)
{
}
ATF_TC_CLEANUP(second, tc)
{
}

ATF_TC_WITHOUT_HEAD(third);
ATF_TC_BODY(third, tc)
{
}

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, first);
    ATF_TP_ADD_TC(tp, second);
    ATF_TP_ADD_TC(tp, third);

    return atf_no_error();
}