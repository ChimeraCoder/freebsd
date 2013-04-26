
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
#include <machine/endian.h>

typedef	const void	*cvp;
typedef	const unsigned char	*ustring;
typedef unsigned long	ul;
typedef const unsigned long	*culp;

/*
 * bcmp -- vax cmpc3 instruction
 */
int
bcmp(b1, b2, length)
	const void *b1, *b2;
	register size_t length;
{
#if BYTE_ORDER == LITTLE_ENDIAN
	/*
	 * The following code is endian specific.  Changing it from
	 * little-endian to big-endian is fairly trivial, but making
	 * it do both is more difficult.
	 *
	 * Note that this code will reference the entire longword which
	 * includes the final byte to compare.  I don't believe this is
	 * a problem since AFAIK, objects are not protected at smaller
	 * than longword boundaries.
	 */
	int	shl, shr, len = length;
	ustring	p1 = b1, p2 = b2;
	ul	va, vb;

	if (len == 0)
		return (0);

	/*
	 * align p1 to a longword boundary
	 */
	while ((long)p1 & (sizeof(long) - 1)) {
		if (*p1++ != *p2++)
			return (1);
		if (--len <= 0)
			return (0);
	}

	/*
	 * align p2 to longword boundary and calculate the shift required to
	 * align p1 and p2
	 */
	shr = (long)p2 & (sizeof(long) - 1);
	if (shr != 0) {
		p2 -= shr;			/* p2 now longword aligned */
		shr <<= 3;			/* offset in bits */
		shl = (sizeof(long) << 3) - shr;

		va = *(culp)p2;
		p2 += sizeof(long);

		while ((len -= sizeof(long)) >= 0) {
			vb = *(culp)p2;
			p2 += sizeof(long);
			if (*(culp)p1 != (va >> shr | vb << shl))
				return (1);
			p1 += sizeof(long);
			va = vb;
		}
		/*
		 * At this point, len is between -sizeof(long) and -1,
		 * representing 0 .. sizeof(long)-1 bytes remaining.
		 */
		if (!(len += sizeof(long)))
			return (0);

		len <<= 3;		/* remaining length in bits */
		/*
		 * The following is similar to the `if' condition
		 * inside the above while loop.  The ?: is necessary
		 * to avoid accessing the longword after the longword
		 * containing the last byte to be compared.
		 */
		return ((((va >> shr | ((shl < len) ? *(culp)p2 << shl : 0)) ^
		    *(culp)p1) & ((1L << len) - 1)) != 0);
	} else {
		/* p1 and p2 have common alignment so no shifting needed */
		while ((len -= sizeof(long)) >= 0) {
			if (*(culp)p1 != *(culp)p2)
				return (1);
			p1 += sizeof(long);
			p2 += sizeof(long);
		}

		/*
		 * At this point, len is between -sizeof(long) and -1,
		 * representing 0 .. sizeof(long)-1 bytes remaining.
		 */
		if (!(len += sizeof(long)))
			return (0);

		return (((*(culp)p1 ^ *(culp)p2)
			 & ((1L << (len << 3)) - 1)) != 0);
	}
#else
	const char *p1, *p2;

	if (length == 0)
		return(0);
	p1 = b1;
	p2 = b2;
	do
		if (*p1++ != *p2++)
			break;
	while (--length);
	return(length);
#endif
}