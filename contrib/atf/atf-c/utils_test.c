
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

#include <stdlib.h>
#include <string.h>

#include <atf-c.h>

#include "atf-c/utils.h"

#include "detail/test_helpers.h"

ATF_TC_WITHOUT_HEAD(free_charpp_empty);
ATF_TC_BODY(free_charpp_empty, tc)
{
    char **array = malloc(sizeof(char *) * 1);
    array[0] = NULL;

    atf_utils_free_charpp(array);
}

ATF_TC_WITHOUT_HEAD(free_charpp_some);
ATF_TC_BODY(free_charpp_some, tc)
{
    char **array = malloc(sizeof(char *) * 4);
    array[0] = strdup("first");
    array[1] = strdup("second");
    array[2] = strdup("third");
    array[3] = NULL;

    atf_utils_free_charpp(array);
}

HEADER_TC(include, "atf-c/utils.h");

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, free_charpp_empty);
    ATF_TP_ADD_TC(tp, free_charpp_some);

    ATF_TP_ADD_TC(tp, include);

    return atf_no_error();
}