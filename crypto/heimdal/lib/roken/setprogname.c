
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

#include <config.h>

#include "roken.h"

#ifndef HAVE___PROGNAME
extern const char *__progname;
#endif

#ifndef HAVE_SETPROGNAME

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
setprogname(const char *argv0)
{

#ifndef HAVE___PROGNAME

    const char *p;
    if(argv0 == NULL)
	return;
    p = strrchr(argv0, '/');

#ifdef BACKSLASH_PATH_DELIM
    {
        const char * pb;

        pb = strrchr((p != NULL)? p : argv0, '\\');
        if (pb != NULL)
            p = pb;
    }
#endif

    if(p == NULL)
	p = argv0;
    else
	p++;

#ifdef _WIN32
    {
        char * fn = strdup(p);
        char * ext;

        strlwr(fn);
        ext = strrchr(fn, '.');
        if (ext != NULL && !strcmp(ext, ".exe"))
            *ext = '\0';

        __progname = fn;
    }
#else

    __progname = p;

#endif

#endif  /* HAVE___PROGNAME */
}

#endif /* HAVE_SETPROGNAME */