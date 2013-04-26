
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

#include <string.h>
#include <unistd.h>

#include <atf-c.h>

#include "detail/test_helpers.h"

ATF_TC(getopt);
ATF_TC_HEAD(getopt, tc)
{
    atf_tc_set_md_var(tc, "descr", "Checks if getopt(3) global state is "
        "reset by the test program driver so that test cases can use "
        "getopt(3) again");
}
ATF_TC_BODY(getopt, tc)
{
    /* Provide an option that is unknown to the test program driver and
     * one that is, together with an argument that would be swallowed by
     * the test program option if it were recognized. */
    int argc = 4;
    char arg1[] = "progname";
    char arg2[] = "-Z";
    char arg3[] = "-s";
    char arg4[] = "foo";
    char *const argv[] = { arg1, arg2, arg3, arg4, NULL };

    int ch;
    bool zflag;

    /* Given that this obviously is a test program, and that we used the
     * same driver to start, we can test getopt(3) right here without doing
     * any fancy stuff. */
    zflag = false;
    while ((ch = getopt(argc, argv, ":Z")) != -1) {
        switch (ch) {
        case 'Z':
            zflag = true;
            break;

        case '?':
        default:
            if (optopt != 's')
                atf_tc_fail("Unexpected unknown option -%c found", optopt);
        }
    }

    ATF_REQUIRE(zflag);
    ATF_REQUIRE_EQ_MSG(1, argc - optind, "Invalid number of arguments left "
        "after the call to getopt(3)");
    ATF_CHECK_STREQ_MSG("foo", argv[optind], "The non-option argument is "
        "invalid");
}

/* ---------------------------------------------------------------------
 * Tests cases for the header file.
 * --------------------------------------------------------------------- */

HEADER_TC(include, "atf-c/tp.h");

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, getopt);

    /* Add the test cases for the header file. */
    ATF_TP_ADD_TC(tp, include);

    return atf_no_error();
}