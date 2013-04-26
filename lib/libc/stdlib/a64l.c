
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
/*-
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */
#if 0
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: a64l.c,v 1.8 2000/01/22 22:19:19 mycroft Exp $");
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdlib.h>
#include <inttypes.h>

#define	ADOT	46		/* ASCII '.' */
#define	ASLASH	47		/* ASCII '/' */
#define	A0	48		/* ASCII '0' */
#define	AA	65		/* ASCII 'A' */
#define	Aa	97		/* ASCII 'a' */

long
a64l(const char *s)
{
	long shift;
	int digit, i, value;

	value = 0;
	shift = 0;
	for (i = 0; *s != '\0' && i < 6; i++, s++) {
		if (*s <= ASLASH)
			digit = *s - ASLASH + 1;
		else if (*s <= A0 + 9)
			digit = *s - A0 + 2;
		else if (*s <= AA + 25)
			digit = *s - AA + 12;
		else
			digit = *s - Aa + 38;

		value |= digit << shift;
		shift += 6;
	}
	return (value);
}