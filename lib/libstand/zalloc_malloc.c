
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

/*
 * MALLOC.C - malloc equivalent, runs on top of zalloc and uses sbrk
 */

#include "zalloc_defs.h"

static MemPool	MallocPool;

#ifdef DMALLOCDEBUG
static int MallocMax;
static int MallocCount;

void mallocstats(void);
#endif

#ifdef malloc
#undef malloc
#undef free
#endif

void *
Malloc(size_t bytes, const char *file, int line)
{
    Guard *res;

#ifdef USEENDGUARD
    bytes += MALLOCALIGN + 1;
#else
    bytes += MALLOCALIGN;
#endif

    while ((res = znalloc(&MallocPool, bytes)) == NULL) {
	int incr = (bytes + BLKEXTENDMASK) & ~BLKEXTENDMASK;
	char *base;

	if ((base = sbrk(incr)) == (char *)-1)
	    return(NULL);
	zextendPool(&MallocPool, base, incr);
	zfree(&MallocPool, base, incr);
    }
#ifdef DMALLOCDEBUG
    if (++MallocCount > MallocMax)
	MallocMax = MallocCount;
#endif
#ifdef USEGUARD
    res->ga_Magic = GAMAGIC;
#endif
    res->ga_Bytes = bytes;
#ifdef USEENDGUARD
    *((signed char *)res + bytes - 1) = -2;
#endif

    return((char *)res + MALLOCALIGN);
}

void
Free(void *ptr, const char *file, int line)
{
    size_t bytes;

    if (ptr != NULL) {
	Guard *res = (void *)((char *)ptr - MALLOCALIGN);

	if (file == NULL)
		file = "unknown";
#ifdef USEGUARD
	if (res->ga_Magic == GAFREE) {
	    printf("free: duplicate free @ %p from %s:%d\n", ptr, file, line);
	    return;
	}
	if (res->ga_Magic != GAMAGIC)
	    panic("free: guard1 fail @ %p from %s:%d", ptr, file, line);
	res->ga_Magic = GAFREE;
#endif
#ifdef USEENDGUARD
	if (*((signed char *)res + res->ga_Bytes - 1) == -1) {
	    printf("free: duplicate2 free @ %p from %s:%d\n", ptr, file, line);
	    return;
	}
	if (*((signed char *)res + res->ga_Bytes - 1) != -2)
	    panic("free: guard2 fail @ %p + %zu from %s:%d", ptr, res->ga_Bytes - MALLOCALIGN, file, line);
	*((signed char *)res + res->ga_Bytes - 1) = -1;
#endif

	bytes = res->ga_Bytes;
	zfree(&MallocPool, res, bytes);
#ifdef DMALLOCDEBUG
	--MallocCount;
#endif
    }
}


void *
Calloc(size_t n1, size_t n2, const char *file, int line)
{
    uintptr_t bytes = (uintptr_t)n1 * (uintptr_t)n2;
    void *res;

    if ((res = Malloc(bytes, file, line)) != NULL) {
	bzero(res, bytes);
#ifdef DMALLOCDEBUG
	if (++MallocCount > MallocMax)
	    MallocMax = MallocCount;
#endif
    }
    return(res);
}

/*
 * realloc() - I could be fancier here and free the old buffer before
 * 	       allocating the new one (saving potential fragmentation
 *	       and potential buffer copies).  But I don't bother.
 */

void *
Realloc(void *ptr, size_t size, const char *file, int line)
{
    void *res;
    size_t old;

    if ((res = Malloc(size, file, line)) != NULL) {
	if (ptr) {
	    old = *(size_t *)((char *)ptr - MALLOCALIGN) - MALLOCALIGN;
	    if (old < size)
		bcopy(ptr, res, old);
	    else
		bcopy(ptr, res, size);
	    Free(ptr, file, line);
	} else {
#ifdef DMALLOCDEBUG
	    if (++MallocCount > MallocMax)
		MallocMax = MallocCount;
#ifdef EXITSTATS
	    if (DidAtExit == 0) {
		DidAtExit = 1;
		atexit(mallocstats);
	    }
#endif
#endif
	}
    }
    return(res);
}

void *
Reallocf(void *ptr, size_t size, const char *file, int line)
{
    void *res;

    if ((res = Realloc(ptr, size, file, line)) == NULL)
	Free(ptr, file, line);
    return(res);
}

#ifdef DMALLOCDEBUG

void
mallocstats(void)
{
    printf("Active Allocations: %d/%d\n", MallocCount, MallocMax);
#ifdef ZALLOCDEBUG
    zallocstats(&MallocPool);
#endif
}

#endif