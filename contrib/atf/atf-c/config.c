
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

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "atf-c/config.h"

#include "detail/env.h"
#include "detail/sanity.h"

static bool initialized = false;

static struct var {
    const char *name;
    const char *default_value;
    const char *value;
    bool can_be_empty;
} vars[] = {
    { "atf_arch",           ATF_ARCH,           NULL, false, },
    { "atf_build_cc",       ATF_BUILD_CC,       NULL, false, },
    { "atf_build_cflags",   ATF_BUILD_CFLAGS,   NULL, true,  },
    { "atf_build_cpp",      ATF_BUILD_CPP,      NULL, false, },
    { "atf_build_cppflags", ATF_BUILD_CPPFLAGS, NULL, true,  },
    { "atf_build_cxx",      ATF_BUILD_CXX,      NULL, false, },
    { "atf_build_cxxflags", ATF_BUILD_CXXFLAGS, NULL, true,  },
    { "atf_confdir",        ATF_CONFDIR,        NULL, false, },
    { "atf_includedir",     ATF_INCLUDEDIR,     NULL, false, },
    { "atf_libdir",         ATF_LIBDIR,         NULL, false, },
    { "atf_libexecdir",     ATF_LIBEXECDIR,     NULL, false, },
    { "atf_machine",        ATF_MACHINE,        NULL, false, },
    { "atf_pkgdatadir",     ATF_PKGDATADIR,     NULL, false, },
    { "atf_shell",          ATF_SHELL,          NULL, false, },
    { "atf_workdir",        ATF_WORKDIR,        NULL, false, },
    { NULL,                 NULL,               NULL, false, },
};

/* Only used for unit testing, so this prototype is private. */
void __atf_config_reinit(void);

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

static
char *
string_to_upper(const char *str)
{
    char *uc;

    uc = (char *)malloc(strlen(str) + 1);
    if (uc != NULL) {
        char *ucptr = uc;
        while (*str != '\0') {
            *ucptr = toupper((int)*str);

            str++;
            ucptr++;
        }
        *ucptr = '\0';
    }

    return uc;
}

static
void
initialize_var(struct var *var, const char *envname)
{
    PRE(var->value == NULL);

    if (atf_env_has(envname)) {
        const char *val = atf_env_get(envname);
        if (strlen(val) > 0 || var->can_be_empty)
            var->value = val;
        else
            var->value = var->default_value;
    } else
        var->value = var->default_value;

    POST(var->value != NULL);
}

static
void
initialize(void)
{
    struct var *var;

    PRE(!initialized);

    for (var = vars; var->name != NULL; var++) {
        char *envname;

        envname = string_to_upper(var->name);
        initialize_var(var, envname);
        free(envname);
    }

    initialized = true;
}

/* ---------------------------------------------------------------------
 * Free functions.
 * --------------------------------------------------------------------- */

const char *
atf_config_get(const char *name)
{
    const struct var *var;
    const char *value;

    if (!initialized) {
        initialize();
        INV(initialized);
    }

    value = NULL;
    for (var = vars; value == NULL && var->name != NULL; var++)
        if (strcmp(var->name, name) == 0)
            value = var->value;
    INV(value != NULL);

    return value;
}

void
__atf_config_reinit(void)
{
    struct var *var;

    initialized = false;

    for (var = vars; var->name != NULL; var++)
        var->value = NULL;
}