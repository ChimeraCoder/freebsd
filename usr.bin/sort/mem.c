
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"

/*
 * malloc() wrapper.
 */
void *
sort_malloc(size_t size)
{
	void *ptr;

	if ((ptr = malloc(size)) == NULL)
		err(2, NULL);
	return (ptr);
}

/*
 * free() wrapper.
 */
void
sort_free(const void *ptr)
{

	if (ptr)
		free(__DECONST(void *, ptr));
}

/*
 * realloc() wrapper.
 */
void *
sort_realloc(void *ptr, size_t size)
{

	if ((ptr = realloc(ptr, size)) == NULL)
		err(2, NULL);
	return (ptr);
}

char *
sort_strdup(const char *str)
{
	char *dup;

	if ((dup = strdup(str)) == NULL)
		err(2, NULL);
	return (dup);
}