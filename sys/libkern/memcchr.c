
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

#include <sys/libkern.h>
#include <sys/limits.h>
#include <sys/param.h>

/*
 * memcchr(): find first character in buffer not matching `c'.
 *
 * This function performs the complement of memchr().  To provide decent
 * performance, this function compares data from the buffer one word at
 * a time.
 *
 * This code is inspired by libc's strlen(), written by Xin Li.
 */

#if LONG_BIT != 32 && LONG_BIT != 64
#error Unsupported word size
#endif

#define	LONGPTR_MASK (sizeof(long) - 1)

#define	TESTBYTE				\
	do {					\
		if (*p != (unsigned char)c)	\
			goto done;		\
		p++;				\
	} while (0)

void *
memcchr(const void *begin, int c, size_t n)
{
	const unsigned long *lp;
	const unsigned char *p, *end;
	unsigned long word;

	/* Four or eight repetitions of `c'. */
	word = (unsigned char)c;
	word |= word << 8;
	word |= word << 16;
#if LONG_BIT >= 64
	word |= word << 32;
#endif

	/* Don't perform memory I/O when passing a zero-length buffer. */
	if (n == 0)
		return (NULL);

	/*
	 * First determine whether there is a character unequal to `c'
	 * in the first word.  As this word may contain bytes before
	 * `begin', we may execute this loop spuriously.
	 */
	lp = (const unsigned long *)((uintptr_t)begin & ~LONGPTR_MASK);
	end = (const unsigned char *)begin + n;
	if (*lp++ != word)
		for (p = begin; p < (const unsigned char *)lp;)
			TESTBYTE;

	/* Now compare the data one word at a time. */
	for (; (const unsigned char *)lp < end; lp++) {
		if (*lp != word) {
			p = (const unsigned char *)lp;
			TESTBYTE;
			TESTBYTE;
			TESTBYTE;
#if LONG_BIT >= 64
			TESTBYTE;
			TESTBYTE;
			TESTBYTE;
			TESTBYTE;
#endif
			goto done;
		}
	}

	return (NULL);

done:
	/*
	 * If the end of the buffer is not word aligned, the previous
	 * loops may obtain an address that's beyond the end of the
	 * buffer.
	 */
	if (p < end)
		return (__DECONST(void *, p));
	return (NULL);
}