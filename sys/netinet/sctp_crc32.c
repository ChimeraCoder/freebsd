
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

#include <netinet/sctp_os.h>
#include <netinet/sctp.h>
#include <netinet/sctp_crc32.h>
#include <netinet/sctp_pcb.h>


#if !defined(SCTP_WITH_NO_CSUM)

static uint32_t
sctp_finalize_crc32c(uint32_t crc32c)
{
	uint32_t result;

#if BYTE_ORDER == BIG_ENDIAN
	uint8_t byte0, byte1, byte2, byte3;

#endif
	/* Complement the result */
	result = ~crc32c;
#if BYTE_ORDER == BIG_ENDIAN
	/*
	 * For BIG-ENDIAN.. aka Motorola byte order the result is in
	 * little-endian form. So we must manually swap the bytes. Then we
	 * can call htonl() which does nothing...
	 */
	byte0 = result & 0x000000ff;
	byte1 = (result >> 8) & 0x000000ff;
	byte2 = (result >> 16) & 0x000000ff;
	byte3 = (result >> 24) & 0x000000ff;
	crc32c = ((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3);
#else
	/*
	 * For INTEL platforms the result comes out in network order. No
	 * htonl is required or the swap above. So we optimize out both the
	 * htonl and the manual swap above.
	 */
	crc32c = result;
#endif
	return (crc32c);
}

uint32_t
sctp_calculate_cksum(struct mbuf *m, uint32_t offset)
{
	/*
	 * given a mbuf chain with a packetheader offset by 'offset'
	 * pointing at a sctphdr (with csum set to 0) go through the chain
	 * of SCTP_BUF_NEXT()'s and calculate the SCTP checksum. This also
	 * has a side bonus as it will calculate the total length of the
	 * mbuf chain. Note: if offset is greater than the total mbuf
	 * length, checksum=1, pktlen=0 is returned (ie. no real error code)
	 */
	uint32_t base = 0xffffffff;
	struct mbuf *at;

	at = m;
	/* find the correct mbuf and offset into mbuf */
	while ((at != NULL) && (offset > (uint32_t) SCTP_BUF_LEN(at))) {
		offset -= SCTP_BUF_LEN(at);	/* update remaining offset
						 * left */
		at = SCTP_BUF_NEXT(at);
	}
	while (at != NULL) {
		if ((SCTP_BUF_LEN(at) - offset) > 0) {
			base = calculate_crc32c(base,
			    (unsigned char *)(SCTP_BUF_AT(at, offset)),
			    (unsigned int)(SCTP_BUF_LEN(at) - offset));
		}
		if (offset) {
			/* we only offset once into the first mbuf */
			if (offset < (uint32_t) SCTP_BUF_LEN(at))
				offset = 0;
			else
				offset -= SCTP_BUF_LEN(at);
		}
		at = SCTP_BUF_NEXT(at);
	}
	base = sctp_finalize_crc32c(base);
	return (base);
}

#endif				/* !defined(SCTP_WITH_NO_CSUM) */


void
sctp_delayed_cksum(struct mbuf *m, uint32_t offset)
{
#if defined(SCTP_WITH_NO_CSUM)
#ifdef INVARIANTS
	panic("sctp_delayed_cksum() called when using no SCTP CRC.");
#endif
#else
	uint32_t checksum;

	checksum = sctp_calculate_cksum(m, offset);
	SCTP_STAT_DECR(sctps_sendhwcrc);
	SCTP_STAT_INCR(sctps_sendswcrc);
	offset += offsetof(struct sctphdr, checksum);

	if (offset + sizeof(uint32_t) > (uint32_t) (m->m_len)) {
		SCTP_PRINTF("sctp_delayed_cksum(): m->len: %d,  off: %d.\n",
		    (uint32_t) m->m_len, offset);
		/*
		 * XXX this shouldn't happen, but if it does, the correct
		 * behavior may be to insert the checksum in the appropriate
		 * next mbuf in the chain.
		 */
		return;
	}
	*(uint32_t *) (m->m_data + offset) = checksum;
#endif
}