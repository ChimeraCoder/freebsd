
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>

/*
 * General routine to allocate a hash table with control of memory flags.
 */
void *
hashinit_flags(int elements, struct malloc_type *type, u_long *hashmask,
    int flags)
{
	long hashsize;
	LIST_HEAD(generic, generic) *hashtbl;
	int i;

	KASSERT(elements > 0, ("%s: bad elements", __func__));
	/* Exactly one of HASH_WAITOK and HASH_NOWAIT must be set. */
	KASSERT((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT),
	    ("Bad flags (0x%x) passed to hashinit_flags", flags));

	for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
		continue;
	hashsize >>= 1;

	if (flags & HASH_NOWAIT)
		hashtbl = malloc((u_long)hashsize * sizeof(*hashtbl),
		    type, M_NOWAIT);
	else
		hashtbl = malloc((u_long)hashsize * sizeof(*hashtbl),
		    type, M_WAITOK);

	if (hashtbl != NULL) {
		for (i = 0; i < hashsize; i++)
			LIST_INIT(&hashtbl[i]);
		*hashmask = hashsize - 1;
	}
	return (hashtbl);
}

/*
 * Allocate and initialize a hash table with default flag: may sleep.
 */
void *
hashinit(int elements, struct malloc_type *type, u_long *hashmask)
{

	return (hashinit_flags(elements, type, hashmask, HASH_WAITOK));
}

void
hashdestroy(void *vhashtbl, struct malloc_type *type, u_long hashmask)
{
	LIST_HEAD(generic, generic) *hashtbl, *hp;

	hashtbl = vhashtbl;
	for (hp = hashtbl; hp <= &hashtbl[hashmask]; hp++)
		KASSERT(LIST_EMPTY(hp), ("%s: hash not empty", __func__));
	free(hashtbl, type);
}

static const int primes[] = { 1, 13, 31, 61, 127, 251, 509, 761, 1021, 1531,
			2039, 2557, 3067, 3583, 4093, 4603, 5119, 5623, 6143,
			6653, 7159, 7673, 8191, 12281, 16381, 24571, 32749 };
#define NPRIMES (sizeof(primes) / sizeof(primes[0]))

/*
 * General routine to allocate a prime number sized hash table.
 */
void *
phashinit(int elements, struct malloc_type *type, u_long *nentries)
{
	long hashsize;
	LIST_HEAD(generic, generic) *hashtbl;
	int i;

	KASSERT(elements > 0, ("%s: bad elements", __func__));
	for (i = 1, hashsize = primes[1]; hashsize <= elements;) {
		i++;
		if (i == NPRIMES)
			break;
		hashsize = primes[i];
	}
	hashsize = primes[i - 1];
	hashtbl = malloc((u_long)hashsize * sizeof(*hashtbl), type, M_WAITOK);
	for (i = 0; i < hashsize; i++)
		LIST_INIT(&hashtbl[i]);
	*nentries = hashsize;
	return (hashtbl);
}