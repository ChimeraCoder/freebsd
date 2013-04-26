
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

#include "config.h"

#if defined(LIBC_SCCS) && !defined(lint)
static const char sccsid[] = "@(#)memset.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>

#include <limits.h>
#include <string.h>

/*
 * PUBLIC: #ifndef HAVE_MEMSET
 * PUBLIC: void *memset __P((void *, int, size_t));
 * PUBLIC: #endif
 */
#define	wsize	sizeof(u_int)
#define	wmask	(wsize - 1)

#ifdef BZERO
#define	RETURN	return
#define	VAL	0
#define	WIDEVAL	0

void
bzero(dst0, length)
	void *dst0;
	register size_t length;
#else
#define	RETURN	return (dst0)
#define	VAL	c0
#define	WIDEVAL	c

void *
memset(dst0, c0, length)
	void *dst0;
	register int c0;
	register size_t length;
#endif
{
	register size_t t;
	register u_int c;
	register u_char *dst;

	dst = dst0;
	/*
	 * If not enough words, just fill bytes.  A length >= 2 words
	 * guarantees that at least one of them is `complete' after
	 * any necessary alignment.  For instance:
	 *
	 *	|-----------|-----------|-----------|
	 *	|00|01|02|03|04|05|06|07|08|09|0A|00|
	 *	          ^---------------------^
	 *		 dst		 dst+length-1
	 *
	 * but we use a minimum of 3 here since the overhead of the code
	 * to do word writes is substantial.
	 */ 
	if (length < 3 * wsize) {
		while (length != 0) {
			*dst++ = VAL;
			--length;
		}
		RETURN;
	}

#ifndef BZERO
	if ((c = (u_char)c0) != 0) {	/* Fill the word. */
		c = (c << 8) | c;	/* u_int is 16 bits. */
#if UINT_MAX > 0xffff
		c = (c << 16) | c;	/* u_int is 32 bits. */
#endif
#if UINT_MAX > 0xffffffff
		c = (c << 32) | c;	/* u_int is 64 bits. */
#endif
	}
#endif
	/* Align destination by filling in bytes. */
	if ((t = (int)dst & wmask) != 0) {
		t = wsize - t;
		length -= t;
		do {
			*dst++ = VAL;
		} while (--t != 0);
	}

	/* Fill words.  Length was >= 2*words so we know t >= 1 here. */
	t = length / wsize;
	do {
		*(u_int *)dst = WIDEVAL;
		dst += wsize;
	} while (--t != 0);

	/* Mop up trailing bytes, if any. */
	t = length & wmask;
	if (t != 0)
		do {
			*dst++ = VAL;
		} while (--t != 0);
	RETURN;
}