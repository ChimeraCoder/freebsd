
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <sys/endian.h>
#include <net/if.h>
#include <net/pfvar.h>	/* XXX */
#include <net/if_pfsync.h>
#include <netinet/ip.h>
#define	TCPSTATES
#include <netinet/tcp_fsm.h>

#include <string.h>

#include "interface.h"
#include "addrtoname.h"

static void	pfsync_print(struct pfsync_header *, const u_char *, u_int);
static void	print_src_dst(const struct pfsync_state_peer *,
		    const struct pfsync_state_peer *, uint8_t);
static void	print_state(struct pfsync_state *);

#ifdef notyet
void
pfsync_if_print(u_char *user, const struct pcap_pkthdr *h,
    register const u_char *p)
{
	u_int caplen = h->caplen;

	ts_print(&h->ts);

	if (caplen < PFSYNC_HDRLEN) {
		printf("[|pfsync]");
		goto out;
	}

	pfsync_print((struct pfsync_header *)p,
	    p + sizeof(struct pfsync_header),
	    caplen - sizeof(struct pfsync_header));
out:
	if (xflag) {
		default_print((const u_char *)p, caplen);
	}
	putchar('\n');
}
#endif /* notyet */

void
pfsync_ip_print(const u_char *bp, u_int len)
{
	struct pfsync_header *hdr = (struct pfsync_header *)bp;

	if (len < PFSYNC_HDRLEN)
		printf("[|pfsync]");
	else
		pfsync_print(hdr, bp + sizeof(struct pfsync_header),
		    len - sizeof(struct pfsync_header));
}

struct pfsync_actions {
	const char *name;
	size_t len;
	void (*print)(const void *);
};

static void	pfsync_print_clr(const void *);
static void	pfsync_print_state(const void *);
static void	pfsync_print_ins_ack(const void *);
static void	pfsync_print_upd_c(const void *);
static void	pfsync_print_upd_req(const void *);
static void	pfsync_print_del_c(const void *);
static void	pfsync_print_bus(const void *);
static void	pfsync_print_tdb(const void *);

struct pfsync_actions actions[] = {
	{ "clear all", sizeof(struct pfsync_clr),	pfsync_print_clr },
	{ "insert", sizeof(struct pfsync_state),	pfsync_print_state },
	{ "insert ack", sizeof(struct pfsync_ins_ack),	pfsync_print_ins_ack },
	{ "update", sizeof(struct pfsync_ins_ack),	pfsync_print_state },
	{ "update compressed", sizeof(struct pfsync_upd_c),
							pfsync_print_upd_c },
	{ "request uncompressed", sizeof(struct pfsync_upd_req),
							pfsync_print_upd_req },
	{ "delete", sizeof(struct pfsync_state),	pfsync_print_state },
	{ "delete compressed", sizeof(struct pfsync_del_c),
							pfsync_print_del_c },
	{ "frag insert", 0,				NULL },
	{ "frag delete", 0,				NULL },
	{ "bulk update status", sizeof(struct pfsync_bus),
							pfsync_print_bus },
	{ "tdb", 0,					pfsync_print_tdb },
	{ "eof", 0,					NULL },
};

static void
pfsync_print(struct pfsync_header *hdr, const u_char *bp, u_int len)
{
	struct pfsync_subheader *subh;
	int count, plen, i;
	u_int alen;

	plen = ntohs(hdr->len);

	printf("PFSYNCv%d len %d", hdr->version, plen);

	if (hdr->version != PFSYNC_VERSION)
		return;

	plen -= sizeof(*hdr);

	while (plen > 0) {
		if (len < sizeof(*subh))
			break;

		subh = (struct pfsync_subheader *)bp;
		bp += sizeof(*subh);
		len -= sizeof(*subh);
		plen -= sizeof(*subh);

		if (subh->action >= PFSYNC_ACT_MAX) {
			printf("\n    act UNKNOWN id %d", subh->action);
			return;
		}

		count = ntohs(subh->count);
		printf("\n    %s count %d", actions[subh->action].name, count);
		alen = actions[subh->action].len;

		if (subh->action == PFSYNC_ACT_EOF)
			return;

		if (actions[subh->action].print == NULL) {
			printf("\n    unimplemented action %hhu", subh->action);
			return;
		}

		for (i = 0; i < count; i++) {
			if (len < alen) {
				len = 0;
				break;
			}

			if (vflag)
				actions[subh->action].print(bp);

			bp += alen;
			len -= alen;
			plen -= alen;
		}
	}

	if (plen > 0) {
		printf("\n    ...");
		return;
	}
	if (plen < 0) {
		printf("\n    invalid header length");
		return;
	}
	if (len > 0)
		printf("\n    invalid packet length");
}

static void
pfsync_print_clr(const void *bp)
{
	const struct pfsync_clr *clr = bp;

	printf("\n\tcreatorid: %08x", htonl(clr->creatorid));
	if (clr->ifname[0] != '\0')
		printf(" interface: %s", clr->ifname);
}

static void
pfsync_print_state(const void *bp)
{
	struct pfsync_state *st = (struct pfsync_state *)bp;

	putchar('\n');
	print_state(st);
}

static void
pfsync_print_ins_ack(const void *bp)
{
	const struct pfsync_ins_ack *iack = bp;

	printf("\n\tid: %016jx creatorid: %08x", (uintmax_t )be64toh(iack->id),
	    ntohl(iack->creatorid));
}

static void
pfsync_print_upd_c(const void *bp)
{
	const struct pfsync_upd_c *u = bp;

	printf("\n\tid: %016jx creatorid: %08x", (uintmax_t )be64toh(u->id),
	    ntohl(u->creatorid));
	if (vflag > 2) {
		printf("\n\tTCP? :");
		print_src_dst(&u->src, &u->dst, IPPROTO_TCP);
	}
}

static void
pfsync_print_upd_req(const void *bp)
{
	const struct pfsync_upd_req *ur = bp;

	printf("\n\tid: %016jx creatorid: %08x", (uintmax_t )be64toh(ur->id),
	    ntohl(ur->creatorid));
}

static void
pfsync_print_del_c(const void *bp)
{
	const struct pfsync_del_c *d = bp;

	printf("\n\tid: %016jx creatorid: %08x", (uintmax_t )be64toh(d->id),
	    ntohl(d->creatorid));
}

static void
pfsync_print_bus(const void *bp)
{
	const struct pfsync_bus *b = bp;
	uint32_t endtime;
	int min, sec;
	const char *status;

	endtime = ntohl(b->endtime);
	sec = endtime % 60;
	endtime /= 60;
	min = endtime % 60;
	endtime /= 60;

	switch (b->status) {
	case PFSYNC_BUS_START:
		status = "start";
		break;
	case PFSYNC_BUS_END:
		status = "end";
		break;
	default:
		status = "UNKNOWN";
		break;
	}

	printf("\n\tcreatorid: %08x age: %.2u:%.2u:%.2u status: %s",
	    htonl(b->creatorid), endtime, min, sec, status);
}

static void
pfsync_print_tdb(const void *bp)
{
	const struct pfsync_tdb *t = bp;

	printf("\n\tspi: 0x%08x rpl: %ju cur_bytes: %ju",
	    ntohl(t->spi), (uintmax_t )be64toh(t->rpl),
	    (uintmax_t )be64toh(t->cur_bytes));
}

static void
print_host(struct pf_addr *addr, uint16_t port, sa_family_t af,
    const char *proto)
{
	char buf[48];

	if (inet_ntop(af, addr, buf, sizeof(buf)) == NULL)
		printf("?");
	else
		printf("%s", buf);

	if (port)
		printf(".%hu", ntohs(port));
}

static void
print_seq(const struct pfsync_state_peer *p)
{
	if (p->seqdiff)
		printf("[%u + %u](+%u)", ntohl(p->seqlo),
		    ntohl(p->seqhi) - ntohl(p->seqlo), ntohl(p->seqdiff));
	else
		printf("[%u + %u]", ntohl(p->seqlo),
		    ntohl(p->seqhi) - ntohl(p->seqlo));
}

static void
print_src_dst(const struct pfsync_state_peer *src,
    const struct pfsync_state_peer *dst, uint8_t proto)
{

	if (proto == IPPROTO_TCP) {
		if (src->state <= TCPS_TIME_WAIT &&
		    dst->state <= TCPS_TIME_WAIT)
			printf("   %s:%s", tcpstates[src->state],
			    tcpstates[dst->state]);
		else if (src->state == PF_TCPS_PROXY_SRC ||
		    dst->state == PF_TCPS_PROXY_SRC)
			printf("   PROXY:SRC");
		else if (src->state == PF_TCPS_PROXY_DST ||
		    dst->state == PF_TCPS_PROXY_DST)
			printf("   PROXY:DST");
		else
			printf("   <BAD STATE LEVELS %u:%u>",
			    src->state, dst->state);
		if (vflag > 1) {
			printf("\n\t");
			print_seq(src);
			if (src->wscale && dst->wscale)
				printf(" wscale %u",
				    src->wscale & PF_WSCALE_MASK);
			printf("  ");
			print_seq(dst);
			if (src->wscale && dst->wscale)
				printf(" wscale %u",
				    dst->wscale & PF_WSCALE_MASK);
		}
	} else if (proto == IPPROTO_UDP && src->state < PFUDPS_NSTATES &&
	    dst->state < PFUDPS_NSTATES) {
		const char *states[] = PFUDPS_NAMES;

		printf("   %s:%s", states[src->state], states[dst->state]);
	} else if (proto != IPPROTO_ICMP && src->state < PFOTHERS_NSTATES &&
	    dst->state < PFOTHERS_NSTATES) {
		/* XXX ICMP doesn't really have state levels */
		const char *states[] = PFOTHERS_NAMES;

		printf("   %s:%s", states[src->state], states[dst->state]);
	} else {
		printf("   %u:%u", src->state, dst->state);
	}
}

static void
print_state(struct pfsync_state *s)
{
	struct pfsync_state_peer *src, *dst;
	struct pfsync_state_key *sk, *nk;
	int min, sec;

	if (s->direction == PF_OUT) {
		src = &s->src;
		dst = &s->dst;
		sk = &s->key[PF_SK_STACK];
		nk = &s->key[PF_SK_WIRE];
		if (s->proto == IPPROTO_ICMP || s->proto == IPPROTO_ICMPV6)
			sk->port[0] = nk->port[0];
	} else {
		src = &s->dst;
		dst = &s->src;
		sk = &s->key[PF_SK_WIRE];
		nk = &s->key[PF_SK_STACK];
		if (s->proto == IPPROTO_ICMP || s->proto == IPPROTO_ICMPV6)
			sk->port[1] = nk->port[1];
	}
	printf("\t%s ", s->ifname);
	printf("proto %u ", s->proto);

	print_host(&nk->addr[1], nk->port[1], s->af, NULL);
	if (PF_ANEQ(&nk->addr[1], &sk->addr[1], s->af) ||
	    nk->port[1] != sk->port[1]) {
		printf(" (");
		print_host(&sk->addr[1], sk->port[1], s->af, NULL);
		printf(")");
	}
	if (s->direction == PF_OUT)
		printf(" -> ");
	else
		printf(" <- ");
	print_host(&nk->addr[0], nk->port[0], s->af, NULL);
	if (PF_ANEQ(&nk->addr[0], &sk->addr[0], s->af) ||
	    nk->port[0] != sk->port[0]) {
		printf(" (");
		print_host(&sk->addr[0], sk->port[0], s->af, NULL);
		printf(")");
	}

	print_src_dst(src, dst, s->proto);

	if (vflag > 1) {
		uint64_t packets[2];
		uint64_t bytes[2];
		uint32_t creation = ntohl(s->creation);
		uint32_t expire = ntohl(s->expire);

		sec = creation % 60;
		creation /= 60;
		min = creation % 60;
		creation /= 60;
		printf("\n\tage %.2u:%.2u:%.2u", creation, min, sec);
		sec = expire % 60;
		expire /= 60;
		min = expire % 60;
		expire /= 60;
		printf(", expires in %.2u:%.2u:%.2u", expire, min, sec);

		bcopy(s->packets[0], &packets[0], sizeof(uint64_t));
		bcopy(s->packets[1], &packets[1], sizeof(uint64_t));
		bcopy(s->bytes[0], &bytes[0], sizeof(uint64_t));
		bcopy(s->bytes[1], &bytes[1], sizeof(uint64_t));
		printf(", %ju:%ju pkts, %ju:%ju bytes",
		    be64toh(packets[0]), be64toh(packets[1]),
		    be64toh(bytes[0]), be64toh(bytes[1]));
		if (s->anchor != ntohl(-1))
			printf(", anchor %u", ntohl(s->anchor));
		if (s->rule != ntohl(-1))
			printf(", rule %u", ntohl(s->rule));
	}
	if (vflag > 1) {
		uint64_t id;

		bcopy(&s->id, &id, sizeof(uint64_t));
		printf("\n\tid: %016jx creatorid: %08x",
		    (uintmax_t )be64toh(id), ntohl(s->creatorid));
	}
}