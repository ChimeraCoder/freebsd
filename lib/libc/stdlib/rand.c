
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rand.c	8.1 (Berkeley) 6/14/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <stdlib.h>
#include "un-namespace.h"

#ifdef TEST
#include <stdio.h>
#endif /* TEST */

static int
do_rand(unsigned long *ctx)
{
#ifdef  USE_WEAK_SEEDING
/*
 * Historic implementation compatibility.
 * The random sequences do not vary much with the seed,
 * even with overflowing.
 */
	return ((*ctx = *ctx * 1103515245 + 12345) % ((u_long)RAND_MAX + 1));
#else   /* !USE_WEAK_SEEDING */
/*
 * Compute x = (7^5 * x) mod (2^31 - 1)
 * without overflowing 31 bits:
 *      (2^31 - 1) = 127773 * (7^5) + 2836
 * From "Random number generators: good ones are hard to find",
 * Park and Miller, Communications of the ACM, vol. 31, no. 10,
 * October 1988, p. 1195.
 */
	long hi, lo, x;

	/* Can't be initialized with 0, so use another value. */
	if (*ctx == 0)
		*ctx = 123459876;
	hi = *ctx / 127773;
	lo = *ctx % 127773;
	x = 16807 * lo - 2836 * hi;
	if (x < 0)
		x += 0x7fffffff;
	return ((*ctx = x) % ((u_long)RAND_MAX + 1));
#endif  /* !USE_WEAK_SEEDING */
}


int
rand_r(unsigned int *ctx)
{
	u_long val = (u_long) *ctx;
	int r = do_rand(&val);

	*ctx = (unsigned int) val;
	return (r);
}


static u_long next = 1;

int
rand()
{
	return (do_rand(&next));
}

void
srand(seed)
u_int seed;
{
	next = seed;
}


/*
 * sranddev:
 *
 * Many programs choose the seed value in a totally predictable manner.
 * This often causes problems.  We seed the generator using pseudo-random
 * data from the kernel.
 */
void
sranddev()
{
	int mib[2];
	size_t len;

	len = sizeof(next);

	mib[0] = CTL_KERN;
	mib[1] = KERN_ARND;
	sysctl(mib, 2, (void *)&next, &len, NULL, 0);
}


#ifdef TEST

main()
{
    int i;
    unsigned myseed;

    printf("seeding rand with 0x19610910: \n");
    srand(0x19610910);

    printf("generating three pseudo-random numbers:\n");
    for (i = 0; i < 3; i++)
    {
	printf("next random number = %d\n", rand());
    }

    printf("generating the same sequence with rand_r:\n");
    myseed = 0x19610910;
    for (i = 0; i < 3; i++)
    {
	printf("next random number = %d\n", rand_r(&myseed));
    }

    return 0;
}

#endif /* TEST */