
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

#include "opt_cpu.h"

#ifdef RDRAND_RNG

#include <sys/param.h>
#include <sys/time.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/selinfo.h>
#include <sys/systm.h>
#include <dev/random/randomdev.h>

#define	RETRY_COUNT	10

static void random_ivy_init(void);
static void random_ivy_deinit(void);
static int random_ivy_read(void *, int);

struct random_systat random_ivy = {
	.ident = "Hardware, Intel IvyBridge+ RNG",
	.init = random_ivy_init,
	.deinit = random_ivy_deinit,
	.read = random_ivy_read,
	.write = (random_write_func_t *)random_null_func,
	.reseed = (random_reseed_func_t *)random_null_func,
	.seeded = 1,
};

static inline int
ivy_rng_store(long *tmp)
{
#ifdef __GNUCLIKE_ASM
	uint32_t count;

	__asm __volatile(
#ifdef __amd64__
	    ".byte\t0x48,0x0f,0xc7,0xf0\n\t" /* rdrand %rax */
	    "jnc\t1f\n\t"
	    "movq\t%%rax,%1\n\t"
	    "movl\t$8,%%eax\n"
#else /* i386 */
	    ".byte\t0x0f,0xc7,0xf0\n\t" /* rdrand %eax */
	    "jnc\t1f\n\t"
	    "movl\t%%eax,%1\n\t"
	    "movl\t$4,%%eax\n"
#endif
	    "1:\n"	/* %eax is cleared by processor on failure */
	    : "=a" (count), "=g" (*tmp) : "a" (0) : "cc");
	return (count);
#else /* __GNUCLIKE_ASM */
	return (0);
#endif
}

static void
random_ivy_init(void)
{
}

void
random_ivy_deinit(void)
{
}

static int
random_ivy_read(void *buf, int c)
{
	char *b;
	long tmp;
	int count, res, retry;

	for (count = c, b = buf; count > 0; count -= res, b += res) {
		for (retry = 0; retry < RETRY_COUNT; retry++) {
			res = ivy_rng_store(&tmp);
			if (res != 0)
				break;
		}
		if (res == 0)
			break;
		if (res > count)
			res = count;
		memcpy(b, &tmp, res);
	}
	return (c - count);
}

#endif