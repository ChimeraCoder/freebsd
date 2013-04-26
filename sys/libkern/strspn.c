
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
#include <sys/types.h>

#define	IDX(c)	((u_char)(c) / LONG_BIT)
#define	BIT(c)	((u_long)1 << ((u_char)(c) % LONG_BIT))

size_t
strspn(const char *s, const char *charset)
{
	/*
	 * NB: idx and bit are temporaries whose use causes gcc 3.4.2 to
	 * generate better code.  Without them, gcc gets a little confused.
	 */
	const char *s1;
	u_long bit;
	u_long tbl[(UCHAR_MAX + 1) / LONG_BIT];
	int idx;

	if(*s == '\0')
		return (0);

#if LONG_BIT == 64	/* always better to unroll on 64-bit architectures */
	tbl[3] = tbl[2] = tbl[1] = tbl[0] = 0;
#else
	for (idx = 0; idx < sizeof(tbl) / sizeof(tbl[0]); idx++)
		tbl[idx] = 0;
#endif
	for (; *charset != '\0'; charset++) {
		idx = IDX(*charset);
		bit = BIT(*charset);
		tbl[idx] |= bit;
	}

	for(s1 = s; ; s1++) {
		idx = IDX(*s1);
		bit = BIT(*s1);
		if ((tbl[idx] & bit) == 0)
			break;
	}
	return (s1 - s);
}