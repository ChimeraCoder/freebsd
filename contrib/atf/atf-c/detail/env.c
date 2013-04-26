
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

#include <errno.h>
#include <stdlib.h>

#include "atf-c/error.h"

#include "env.h"
#include "sanity.h"
#include "text.h"

const char *
atf_env_get(const char *name)
{
    const char* val = getenv(name);
    PRE(val != NULL);
    return val;
}

bool
atf_env_has(const char *name)
{
    return getenv(name) != NULL;
}

atf_error_t
atf_env_set(const char *name, const char *val)
{
    atf_error_t err;

#if defined(HAVE_SETENV)
    if (setenv(name, val, 1) == -1)
        err = atf_libc_error(errno, "Cannot set environment variable "
                             "'%s' to '%s'", name, val);
    else
        err = atf_no_error();
#elif defined(HAVE_PUTENV)
    char *buf;

    err = atf_text_format(&buf, "%s=%s", name, val);
    if (!atf_is_error(err)) {
        if (putenv(buf) == -1)
            err = atf_libc_error(errno, "Cannot set environment variable "
                                 "'%s' to '%s'", name, val);
        free(buf);
    }
#else
#   error "Don't know how to set an environment variable."
#endif

    return err;
}

atf_error_t
atf_env_unset(const char *name)
{
    atf_error_t err;

#if defined(HAVE_UNSETENV)
    unsetenv(name);
    err = atf_no_error();
#elif defined(HAVE_PUTENV)
    char *buf;

    err = atf_text_format(&buf, "%s=", name);
    if (!atf_is_error(err)) {
        if (putenv(buf) == -1)
            err = atf_libc_error(errno, "Cannot unset environment variable"
                                 " '%s'", name);
        free(buf);
    }
#else
#   error "Don't know how to unset an environment variable."
#endif

    return err;
}