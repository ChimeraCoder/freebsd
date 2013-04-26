
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
#include <sys/types.h>
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#define	_SEARCH_PRIVATE
#include <search.h>
#include <stdint.h>	/* for uint8_t */
#include <stdlib.h>	/* for NULL */
#include <string.h>	/* for memcpy() prototype */

static void *lwork(const void *, const void *, size_t *, size_t,
    int (*)(const void *, const void *), int);

void *lsearch(const void *key, void *base, size_t *nelp, size_t width,
    int (*compar)(const void *, const void *))
{

	return (lwork(key, base, nelp, width, compar, 1));
}

void *lfind(const void *key, const void *base, size_t *nelp, size_t width,
    int (*compar)(const void *, const void *))
{

	return (lwork(key, base, nelp, width, compar, 0));
}

static void *
lwork(const void *key, const void *base, size_t *nelp, size_t width,
    int (*compar)(const void *, const void *), int addelem)
{
	uint8_t *ep, *endp;

	/*
	 * Cast to an integer value first to avoid the warning for removing
	 * 'const' via a cast.
	 */
	ep = (uint8_t *)(uintptr_t)base;
	for (endp = (uint8_t *)(ep + width * *nelp); ep < endp; ep += width) {
		if (compar(key, ep) == 0)
			return (ep);
	}

	/* lfind() shall return when the key was not found. */
	if (!addelem)
		return (NULL);

	/*
	 * lsearch() adds the key to the end of the table and increments
	 * the number of elements.
	 */
	memcpy(endp, key, width);
	++*nelp;

	return (endp);
}