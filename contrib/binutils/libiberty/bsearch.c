
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

/*

@deftypefn Supplemental void* bsearch (const void *@var{key}, const void *@var{base}, size_t @var{nmemb}, size_t @var{size}, int (*@var{compar})(const void *, const void *))

Performs a search over an array of @var{nmemb} elements pointed to by
@var{base} for a member that matches the object pointed to by @var{key}.
The size of each member is specified by @var{size}.  The array contents
should be sorted in ascending order according to the @var{compar}
comparison function.  This routine should take two arguments pointing to
the @var{key} and to an array member, in that order, and should return an
integer less than, equal to, or greater than zero if the @var{key} object
is respectively less than, matching, or greater than the array member.

@end deftypefn

*/

#include "config.h"
#include "ansidecl.h"
#include <sys/types.h>		/* size_t */
#include <stdio.h>

/*
 * Perform a binary search.
 *
 * The code below is a bit sneaky.  After a comparison fails, we
 * divide the work in half by moving either left or right. If lim
 * is odd, moving left simply involves halving lim: e.g., when lim
 * is 5 we look at item 2, so we change lim to 2 so that we will
 * look at items 0 & 1.  If lim is even, the same applies.  If lim
 * is odd, moving right again involes halving lim, this time moving
 * the base up one item past p: e.g., when lim is 5 we change base
 * to item 3 and make lim 2 so that we will look at items 3 and 4.
 * If lim is even, however, we have to shrink it by one before
 * halving: e.g., when lim is 4, we still looked at item 2, so we
 * have to make lim 3, then halve, obtaining 1, so that we will only
 * look at item 3.
 */
void *
bsearch (register const void *key, const void *base0,
         size_t nmemb, register size_t size,
         register int (*compar)(const void *, const void *))
{
	register const char *base = (const char *) base0;
	register int lim, cmp;
	register const void *p;

	for (lim = nmemb; lim != 0; lim >>= 1) {
		p = base + (lim >> 1) * size;
		cmp = (*compar)(key, p);
		if (cmp == 0)
			return (void *)p;
		if (cmp > 0) {	/* key > p: move right */
			base = (const char *)p + size;
			lim--;
		} /* else move left */
	}
	return (NULL);
}