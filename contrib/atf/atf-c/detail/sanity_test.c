
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

#if defined(HAVE_CONFIG_H)
#include "bconfig.h"
#endif

#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <atf-c.h>

#include "dynstr.h"
#include "process.h"
#include "sanity.h"
#include "test_helpers.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

enum type { inv, pre, post, unreachable };

static
bool
grep(const atf_dynstr_t *line, const char *text)
{
    const char *l = atf_dynstr_cstring(line);
    bool found;

    found = false;

    if (strstr(l, text) != NULL)
        found = true;

    return found;
}

struct test_data {
    enum type m_type;
    bool m_cond;
};

static void do_test_child(void *) ATF_DEFS_ATTRIBUTE_NORETURN;

static
void
do_test_child(void *v)
{
    struct test_data *td = v;

    switch (td->m_type) {
    case inv:
        INV(td->m_cond);
        break;

    case pre:
        PRE(td->m_cond);
        break;

    case post:
        POST(td->m_cond);
        break;

    case unreachable:
        if (!td->m_cond)
            UNREACHABLE;
        break;
    }

    exit(EXIT_SUCCESS);
}

static
void
do_test(enum type t, bool cond)
{
    atf_process_child_t child;
    atf_process_status_t status;
    bool eof;
    int nlines;
    atf_dynstr_t lines[3];

    {
        atf_process_stream_t outsb, errsb;
        struct test_data td = { t, cond };

        RE(atf_process_stream_init_inherit(&outsb));
        RE(atf_process_stream_init_capture(&errsb));
        RE(atf_process_fork(&child, do_test_child, &outsb, &errsb, &td));
        atf_process_stream_fini(&errsb);
        atf_process_stream_fini(&outsb);
    }

    nlines = 0;
    eof = false;
    do {
        RE(atf_dynstr_init(&lines[nlines]));
        if (!eof)
            eof = read_line(atf_process_child_stderr(&child), &lines[nlines]);
        nlines++;
    } while (nlines < 3);
    ATF_REQUIRE(nlines == 0 || nlines == 3);

    RE(atf_process_child_wait(&child, &status));
    if (!cond) {
        ATF_REQUIRE(atf_process_status_signaled(&status));
        ATF_REQUIRE(atf_process_status_termsig(&status) == SIGABRT);
    } else {
        ATF_REQUIRE(atf_process_status_exited(&status));
        ATF_REQUIRE(atf_process_status_exitstatus(&status) == EXIT_SUCCESS);
    }
    atf_process_status_fini(&status);

    if (!cond) {
        switch (t) {
        case inv:
            ATF_REQUIRE(grep(&lines[0], "Invariant"));
            break;

        case pre:
            ATF_REQUIRE(grep(&lines[0], "Precondition"));
            break;

        case post:
            ATF_REQUIRE(grep(&lines[0], "Postcondition"));
            break;

        case unreachable:
            ATF_REQUIRE(grep(&lines[0], "Invariant"));
            break;
        }

        ATF_REQUIRE(grep(&lines[0], __FILE__));
        ATF_REQUIRE(grep(&lines[2], PACKAGE_BUGREPORT));
    }

    while (nlines > 0) {
        nlines--;
        atf_dynstr_fini(&lines[nlines]);
    }
}

static
void
require_ndebug(void)
{
#if defined(NDEBUG)
    atf_tc_skip("Sanity checks not available; code built with -DNDEBUG");
#endif
}

/* ---------------------------------------------------------------------
 * Test cases for the free functions.
 * --------------------------------------------------------------------- */

ATF_TC(inv);
ATF_TC_HEAD(inv, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the INV macro");
}
ATF_TC_BODY(inv, tc)
{
    require_ndebug();

    do_test(inv, false);
    do_test(inv, true);
}

ATF_TC(pre);
ATF_TC_HEAD(pre, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the PRE macro");
}
ATF_TC_BODY(pre, tc)
{
    require_ndebug();

    do_test(pre, false);
    do_test(pre, true);
}

ATF_TC(post);
ATF_TC_HEAD(post, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the POST macro");
}
ATF_TC_BODY(post, tc)
{
    require_ndebug();

    do_test(post, false);
    do_test(post, true);
}

ATF_TC(unreachable);
ATF_TC_HEAD(unreachable, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the UNREACHABLE macro");
}
ATF_TC_BODY(unreachable, tc)
{
    require_ndebug();

    do_test(unreachable, false);
    do_test(unreachable, true);
}

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, inv);
    ATF_TP_ADD_TC(tp, pre);
    ATF_TP_ADD_TC(tp, post);
    ATF_TP_ADD_TC(tp, unreachable);

    return atf_no_error();
}