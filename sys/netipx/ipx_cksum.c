
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
#include <sys/mbuf.h>
#include <sys/libkern.h>

#include <netipx/ipx.h>
#include <netipx/ipx_var.h>


#define SUMADV	sum += *w++

u_short
ipx_cksum(struct mbuf *m, int len)
{
	u_int32_t sum = 0;
	u_char *w;
	u_char oldtc;
	int mlen, words;
	struct ipx *ipx;
	union {
		u_char	b[2];
		u_short	w;
	} buf;

	ipx = mtod(m, struct ipx*);
	oldtc = ipx->ipx_tc;
	ipx->ipx_tc = 0;
	w = (u_char *)&ipx->ipx_len;
	len -= 2;
	mlen = 2;

	for(;;) {
		mlen = imin(m->m_len - mlen, len);
		words = mlen / 2;
		len -= mlen & ~1;
		while (words >= 16) {
			SUMADV;	SUMADV;	SUMADV;	SUMADV;
			SUMADV;	SUMADV;	SUMADV;	SUMADV;
			SUMADV;	SUMADV;	SUMADV;	SUMADV;
			SUMADV;	SUMADV;	SUMADV;	SUMADV;
			words -= 16;
		}
		while (words--)
			SUMADV;
		if (len == 0)
			break;
		mlen &= 1;
		if (mlen) {
			buf.b[0] = *w;
			if (--len == 0) {
				buf.b[1] = 0;
				sum += buf.w;
				break;
			}
		}
		m = m->m_next;
		if (m == NULL)
			break;
		w = mtod(m, u_char *);
		if (mlen) {
			buf.b[1] = *w;
			sum += buf.w;
			w++;
			if (--len == 0)
				break;
		}
	}

	ipx->ipx_tc = oldtc;

	sum = (sum & 0xffff) + (sum >> 16);
	if (sum >= 0x10000)
		sum++;
	if (sum)
		sum = ~sum;
	return (sum);
}