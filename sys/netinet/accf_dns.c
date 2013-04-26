
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

#define ACCEPT_FILTER_MOD

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/module.h>
#include <sys/signalvar.h>
#include <sys/sysctl.h>
#include <sys/socketvar.h>

/* check for full DNS request */
static int sohasdns(struct socket *so, void *arg, int waitflag);

struct packet {
	struct mbuf *m;		/* Current mbuf. */
	struct mbuf *n;		/* nextpkt mbuf. */
	unsigned long moff;	/* Offset of the beginning of m. */
	unsigned long offset;	/* Which offset we are working at. */
	unsigned long len;	/* The number of bytes we have to play with. */
};

#define DNS_OK 0
#define DNS_WAIT -1
#define DNS_RUN -2

/* check we can skip over various parts of DNS request */
static int skippacket(struct sockbuf *sb);

static struct accept_filter accf_dns_filter = {
	"dnsready",
	sohasdns,
	NULL,
	NULL
};

static moduledata_t accf_dns_mod = {
	"accf_dns",
	accept_filt_generic_mod_event,
	&accf_dns_filter
};

DECLARE_MODULE(accf_dns, accf_dns_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);

static int
sohasdns(struct socket *so, void *arg, int waitflag)
{
	struct sockbuf *sb = &so->so_rcv;

	/* If the socket is full, we're ready. */
	if (sb->sb_cc >= sb->sb_hiwat || sb->sb_mbcnt >= sb->sb_mbmax)
		goto ready;

	/* Check to see if we have a request. */
	if (skippacket(sb) == DNS_WAIT)
		return (SU_OK);

ready:
	return (SU_ISCONNECTED);
}

#define GET8(p, val) do { \
	if (p->offset < p->moff) \
		return DNS_RUN; \
	while (p->offset >= p->moff + p->m->m_len) { \
		p->moff += p->m->m_len; \
		p->m = p->m->m_next; \
		if (p->m == NULL) { \
			p->m = p->n; \
			p->n = p->m->m_nextpkt; \
		} \
		if (p->m == NULL) \
			return DNS_WAIT; \
	} \
	val = *(mtod(p->m, unsigned char *) + (p->offset - p->moff)); \
	p->offset++; \
	} while (0)

#define GET16(p, val) do { \
	unsigned int v0, v1; \
	GET8(p, v0); \
	GET8(p, v1); \
	val = v0 * 0x100 + v1; \
	} while (0)

static int
skippacket(struct sockbuf *sb) {
	unsigned long packlen;
	struct packet q, *p = &q;

	if (sb->sb_cc < 2)
		return DNS_WAIT;

	q.m = sb->sb_mb;
	q.n = q.m->m_nextpkt;
	q.moff = 0;
	q.offset = 0;
	q.len = sb->sb_cc;

	GET16(p, packlen);
	if (packlen + 2 > q.len)
		return DNS_WAIT;

	return DNS_OK;
}