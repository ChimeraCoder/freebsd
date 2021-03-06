
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

#include <stdio.h>
#include <string.h>

#include <atf-c.h>

#include "atf-c/config.h"

#include "detail/env.h"
#include "detail/test_helpers.h"

static const char *test_value = "env-value";

static struct varnames {
    const char *lc;
    const char *uc;
    bool can_be_empty;
} all_vars[] = {
    { "atf_arch",           "ATF_ARCH",           false },
    { "atf_build_cc",       "ATF_BUILD_CC",       false },
    { "atf_build_cflags",   "ATF_BUILD_CFLAGS",   true  },
    { "atf_build_cpp",      "ATF_BUILD_CPP",      false },
    { "atf_build_cppflags", "ATF_BUILD_CPPFLAGS", true  },
    { "atf_build_cxx",      "ATF_BUILD_CXX",      false },
    { "atf_build_cxxflags", "ATF_BUILD_CXXFLAGS", true  },
    { "atf_confdir",        "ATF_CONFDIR",        false },
    { "atf_includedir",     "ATF_INCLUDEDIR",     false },
    { "atf_libdir",         "ATF_LIBDIR",         false },
    { "atf_libexecdir",     "ATF_LIBEXECDIR",     false },
    { "atf_machine",        "ATF_MACHINE",        false },
    { "atf_pkgdatadir",     "ATF_PKGDATADIR",     false },
    { "atf_shell",          "ATF_SHELL",          false },
    { "atf_workdir",        "ATF_WORKDIR",        false },
    { NULL,                 NULL,                 false }
};

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

void __atf_config_reinit(void);

static
void
unset_all(void)
{
    const struct varnames *v;
    for (v = all_vars; v->lc != NULL; v++)
        RE(atf_env_unset(v->uc));
}

static
void
compare_one(const char *var, const char *expvalue)
{
    const struct varnames *v;

    printf("Checking that %s is set to %s\n", var, expvalue);

    for (v = all_vars; v->lc != NULL; v++) {
        if (strcmp(v->lc, var) == 0)
            ATF_CHECK_STREQ(atf_config_get(v->lc), test_value);
        else
            ATF_CHECK(strcmp(atf_config_get(v->lc), test_value) != 0);
    }
}

/* ---------------------------------------------------------------------
 * Test cases for the free functions.
 * --------------------------------------------------------------------- */

ATF_TC(get);
ATF_TC_HEAD(get, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the atf_config_get function");
}
ATF_TC_BODY(get, tc)
{
    const struct varnames *v;

    /* Unset all known environment variables and make sure the built-in
     * values do not match the bogus value we will use for testing. */
    unset_all();
    __atf_config_reinit();
    for (v = all_vars; v->lc != NULL; v++)
        ATF_CHECK(strcmp(atf_config_get(v->lc), test_value) != 0);

    /* Test the behavior of empty values. */
    for (v = all_vars; v->lc != NULL; v++) {
        unset_all();
        if (strcmp(atf_config_get(v->lc), "") != 0) {
            RE(atf_env_set(v->uc, ""));
            __atf_config_reinit();
            if (v->can_be_empty)
                ATF_CHECK(strlen(atf_config_get(v->lc)) == 0);
            else
                ATF_CHECK(strlen(atf_config_get(v->lc)) > 0);
        }
    }

    /* Check if every variable is recognized individually. */
    for (v = all_vars; v->lc != NULL; v++) {
        unset_all();
        RE(atf_env_set(v->uc, test_value));
        __atf_config_reinit();
        compare_one(v->lc, test_value);
    }
}

/* ---------------------------------------------------------------------
 * Tests cases for the header file.
 * --------------------------------------------------------------------- */

HEADER_TC(include, "atf-c/config.h");

/* ---------------------------------------------------------------------
 * Main.
 * --------------------------------------------------------------------- */

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, get);

    /* Add the test cases for the header file. */
    ATF_TP_ADD_TC(tp, include);

    return atf_no_error();
}