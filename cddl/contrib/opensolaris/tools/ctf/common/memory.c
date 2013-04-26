
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * Routines for memory management
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "memory.h"

static void
memory_bailout(void)
{
	(void) fprintf(stderr, "Out of memory\n");
	exit(1);
}

void *
xmalloc(size_t size)
{
	void *mem;

	if ((mem = malloc(size)) == NULL)
		memory_bailout();

	return (mem);
}

void *
xcalloc(size_t size)
{
	void *mem;

	mem = xmalloc(size);
	bzero(mem, size);

	return (mem);
}

char *
xstrdup(const char *str)
{
	char *newstr;

	if ((newstr = strdup(str)) == NULL)
		memory_bailout();

	return (newstr);
}

char *
xstrndup(char *str, size_t len)
{
	char *newstr;

	if ((newstr = malloc(len + 1)) == NULL)
		memory_bailout();

	(void) strncpy(newstr, str, len);
	newstr[len] = '\0';

	return (newstr);
}

void *
xrealloc(void *ptr, size_t size)
{
	void *mem;

	if ((mem = realloc(ptr, size)) == NULL)
		memory_bailout();

	return (mem);
}