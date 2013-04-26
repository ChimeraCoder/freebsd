
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

#include "includes.h"

#ifdef HAVE_CYGWIN

#if defined(open) && open == binary_open
# undef open
#endif

#include <sys/types.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>

#include "xmalloc.h"

int 
binary_open(const char *filename, int flags, ...)
{
	va_list ap;
	mode_t mode;
	
	va_start(ap, flags);
	mode = va_arg(ap, mode_t);
	va_end(ap);
	return (open(filename, flags | O_BINARY, mode));
}

int
check_ntsec(const char *filename)
{
	return (pathconf(filename, _PC_POSIX_PERMISSIONS));
}

#define NL(x) x, (sizeof (x) - 1)
#define WENV_SIZ (sizeof (wenv_arr) / sizeof (wenv_arr[0]))

static struct wenv {
	const char *name;
	size_t namelen;
} wenv_arr[] = {
	{ NL("ALLUSERSPROFILE=") },
	{ NL("COMPUTERNAME=") },
	{ NL("COMSPEC=") },
	{ NL("CYGWIN=") },
	{ NL("OS=") },
	{ NL("PATH=") },
	{ NL("PATHEXT=") },
	{ NL("PROGRAMFILES=") },
	{ NL("SYSTEMDRIVE=") },
	{ NL("SYSTEMROOT=") },
	{ NL("WINDIR=") }
};

char **
fetch_windows_environment(void)
{
	char **e, **p;
	unsigned int i, idx = 0;

	p = xcalloc(WENV_SIZ + 1, sizeof(char *));
	for (e = environ; *e != NULL; ++e) {
		for (i = 0; i < WENV_SIZ; ++i) {
			if (!strncmp(*e, wenv_arr[i].name, wenv_arr[i].namelen))
				p[idx++] = *e;
		}
	}
	p[idx] = NULL;
	return p;
}

void
free_windows_environment(char **p)
{
	xfree(p);
}

#endif /* HAVE_CYGWIN */