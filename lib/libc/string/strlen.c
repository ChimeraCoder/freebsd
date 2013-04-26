
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

#include <sys/limits.h>
#include <sys/types.h>
#include <string.h>

/*
 * Portable strlen() for 32-bit and 64-bit systems.
 *
 * Rationale: it is generally much more efficient to do word length
 * operations and avoid branches on modern computer systems, as
 * compared to byte-length operations with a lot of branches.
 *
 * The expression:
 *
 *	((x - 0x01....01) & ~x & 0x80....80)
 *
 * would evaluate to a non-zero value iff any of the bytes in the
 * original word is zero.
 *
 * On multi-issue processors, we can divide the above expression into:
 *	a)  (x - 0x01....01)
 *	b) (~x & 0x80....80)
 *	c) a & b
 *
 * Where, a) and b) can be partially computed in parallel.
 *
 * The algorithm above is found on "Hacker's Delight" by
 * Henry S. Warren, Jr.
 */

/* Magic numbers for the algorithm */
#if LONG_BIT == 32
static const unsigned long mask01 = 0x01010101;
static const unsigned long mask80 = 0x80808080;
#elif LONG_BIT == 64
static const unsigned long mask01 = 0x0101010101010101;
static const unsigned long mask80 = 0x8080808080808080;
#else
#error Unsupported word size
#endif

#define	LONGPTR_MASK (sizeof(long) - 1)

/*
 * Helper macro to return string length if we caught the zero
 * byte.
 */
#define testbyte(x)				\
	do {					\
		if (p[x] == '\0')		\
		    return (p - str + x);	\
	} while (0)

size_t
strlen(const char *str)
{
	const char *p;
	const unsigned long *lp;
	long va, vb;

	/*
	 * Before trying the hard (unaligned byte-by-byte access) way
	 * to figure out whether there is a nul character, try to see
	 * if there is a nul character is within this accessible word
	 * first.
	 *
	 * p and (p & ~LONGPTR_MASK) must be equally accessible since
	 * they always fall in the same memory page, as long as page
	 * boundaries is integral multiple of word size.
	 */
	lp = (const unsigned long *)((uintptr_t)str & ~LONGPTR_MASK);
	va = (*lp - mask01);
	vb = ((~*lp) & mask80);
	lp++;
	if (va & vb)
		/* Check if we have \0 in the first part */
		for (p = str; p < (const char *)lp; p++)
			if (*p == '\0')
				return (p - str);

	/* Scan the rest of the string using word sized operation */
	for (; ; lp++) {
		va = (*lp - mask01);
		vb = ((~*lp) & mask80);
		if (va & vb) {
			p = (const char *)(lp);
			testbyte(0);
			testbyte(1);
			testbyte(2);
			testbyte(3);
#if (LONG_BIT >= 64)
			testbyte(4);
			testbyte(5);
			testbyte(6);
			testbyte(7);
#endif
		}
	}

	/* NOTREACHED */
	return (0);
}