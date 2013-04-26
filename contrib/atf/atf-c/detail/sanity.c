
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

#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "sanity.h"

static
void
fail(const char *fmt, ...)
{
    va_list ap;
    char buf[4096];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    warnx("%s", buf);
    warnx("%s", "");
    warnx("This is probably a bug in this application or one of the "
          "libraries it uses.  If you believe this problem is caused "
          "by, or is related to " PACKAGE_STRING ", please report it "
          "to " PACKAGE_BUGREPORT " and provide as many detatils as "
          "possible describing how you got to this condition.");

    abort();
}

void
atf_sanity_inv(const char *file, int line, const char *cond)
{
    fail("Invariant check failed at %s:%d: %s", file, line, cond);
}

void
atf_sanity_pre(const char *file, int line, const char *cond)
{
    fail("Precondition check failed at %s:%d: %s", file, line, cond);
}

void
atf_sanity_post(const char *file, int line, const char *cond)
{
    fail("Postcondition check failed at %s:%d: %s", file, line, cond);
}