
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
#include "mibII.h"
#include "mibII_oid.h"
#include <sys/socketvar.h>
#include <netinet/in_pcb.h>
#include <netinet/udp.h>
#include <netinet/ip_var.h>
#include <netinet/udp_var.h>

struct udp_index {
	struct asn_oid	index;
	struct xinpcb	*inp;
};

static uint64_t udp_tick;
static struct udpstat udpstat;
static struct xinpgen *xinpgen;
static size_t xinpgen_len;
static u_int udp_total;

static u_int oidnum;
static struct udp_index *udpoids;

static int
udp_compare(const void *p1, const void *p2)
{
	const struct udp_index *t1 = p1;
	const struct udp_index *t2 = p2;

	return (asn_compare_oid(&t1->index, &t2->index));
}

static int
fetch_udp(void)
{
	size_t len;
	struct xinpgen *ptr;
	struct xinpcb *inp;
	struct udp_index *oid;
	in_addr_t inaddr;

	len = sizeof(udpstat);
	if (sysctlbyname("net.inet.udp.stats", &udpstat, &len, NULL, 0) == -1) {
		syslog(LOG_ERR, "net.inet.udp.stats: %m");
		return (-1);
	}
	if (len != sizeof(udpstat)) {
		syslog(LOG_ERR, "net.inet.udp.stats: wrong size");
		return (-1);
	}

	udp_tick = get_ticks();

	len = 0;
	if (sysctlbyname("net.inet.udp.pcblist", NULL, &len, NULL, 0) == -1) {
		syslog(LOG_ERR, "net.inet.udp.pcblist: %m");
		return (-1);
	}
	if (len > xinpgen_len) {
		if ((ptr = realloc(xinpgen, len)) == NULL) {
			syslog(LOG_ERR, "%zu: %m", len);
			return (-1);
		}
		xinpgen = ptr;
		xinpgen_len = len;
	}
	if (sysctlbyname("net.inet.udp.pcblist", xinpgen, &len, NULL, 0) == -1) {
		syslog(LOG_ERR, "net.inet.udp.pcblist: %m");
		return (-1);
	}

	udp_total = 0;
	for (ptr = (struct xinpgen *)(void *)((char *)xinpgen + xinpgen->xig_len);
	     ptr->xig_len > sizeof(struct xinpgen);
             ptr = (struct xinpgen *)(void *)((char *)ptr + ptr->xig_len)) {
		inp = (struct xinpcb *)ptr;
		if (inp->xi_inp.inp_gencnt > xinpgen->xig_gen ||
		    (inp->xi_inp.inp_vflag & INP_IPV4) == 0)
			continue;

		udp_total++;
	}

	if (oidnum < udp_total) {
		oid = realloc(udpoids, udp_total * sizeof(udpoids[0]));
		if (oid == NULL) {
			free(udpoids);
			oidnum = 0;
			return (0);
		}
		udpoids = oid;
		oidnum = udp_total;
	}

	oid = udpoids;
	for (ptr = (struct xinpgen *)(void *)((char *)xinpgen + xinpgen->xig_len);
	     ptr->xig_len > sizeof(struct xinpgen);
             ptr = (struct xinpgen *)(void *)((char *)ptr + ptr->xig_len)) {
		inp = (struct xinpcb *)ptr;
		if (inp->xi_inp.inp_gencnt > xinpgen->xig_gen ||
		    (inp->xi_inp.inp_vflag & INP_IPV4) == 0)
			continue;
		oid->inp = inp;
		oid->index.len = 5;
		inaddr = ntohl(inp->xi_inp.inp_laddr.s_addr);
		oid->index.subs[0] = (inaddr >> 24) & 0xff;
		oid->index.subs[1] = (inaddr >> 16) & 0xff;
		oid->index.subs[2] = (inaddr >>  8) & 0xff;
		oid->index.subs[3] = (inaddr >>  0) & 0xff;
		oid->index.subs[4] = ntohs(inp->xi_inp.inp_lport);
		oid++;
	}

	qsort(udpoids, udp_total, sizeof(udpoids[0]), udp_compare);

	return (0);
}

int
op_udp(struct snmp_context *ctx __unused, struct snmp_value *value,
    u_int sub, u_int iidx __unused, enum snmp_op op)
{
	switch (op) {

	  case SNMP_OP_GETNEXT:
		abort();

	  case SNMP_OP_GET:
		break;

	  case SNMP_OP_SET:
		return (SNMP_ERR_NOT_WRITEABLE);

	  case SNMP_OP_ROLLBACK:
	  case SNMP_OP_COMMIT:
		abort();
	}

	if (udp_tick < this_tick)
		if (fetch_udp() == -1)
			return (SNMP_ERR_GENERR);

	switch (value->var.subs[sub - 1]) {

	  case LEAF_udpInDatagrams:
		value->v.uint32 = udpstat.udps_ipackets;
		break;

	  case LEAF_udpNoPorts:
		value->v.uint32 = udpstat.udps_noport +
		    udpstat.udps_noportbcast +
		    udpstat.udps_noportmcast;
		break;

	  case LEAF_udpInErrors:
		value->v.uint32 = udpstat.udps_hdrops +
		    udpstat.udps_badsum +
		    udpstat.udps_badlen +
		    udpstat.udps_fullsock;
		break;

	  case LEAF_udpOutDatagrams:
		value->v.uint32 = udpstat.udps_opackets;
		break;
	}
	return (SNMP_ERR_NOERROR);
}

int
op_udptable(struct snmp_context *ctx __unused, struct snmp_value *value,
    u_int sub, u_int iidx __unused, enum snmp_op op)
{
	u_int i;

	if (udp_tick < this_tick)
		if (fetch_udp() == -1)
			return (SNMP_ERR_GENERR);

	switch (op) {

	  case SNMP_OP_GETNEXT:
		for (i = 0; i < udp_total; i++)
			if (index_compare(&value->var, sub, &udpoids[i].index) < 0)
				break;
		if (i == udp_total)
			return (SNMP_ERR_NOSUCHNAME);
		index_append(&value->var, sub, &udpoids[i].index);
		break;

	  case SNMP_OP_GET:
		for (i = 0; i < udp_total; i++)
			if (index_compare(&value->var, sub, &udpoids[i].index) == 0)
				break;
		if (i == udp_total)
			return (SNMP_ERR_NOSUCHNAME);
		break;

	  case SNMP_OP_SET:
		return (SNMP_ERR_NOT_WRITEABLE);

	  case SNMP_OP_ROLLBACK:
	  case SNMP_OP_COMMIT:
	  default:
		abort();
	}

	switch (value->var.subs[sub - 1]) {

	  case LEAF_udpLocalAddress:
		value->v.ipaddress[0] = udpoids[i].index.subs[0];
		value->v.ipaddress[1] = udpoids[i].index.subs[1];
		value->v.ipaddress[2] = udpoids[i].index.subs[2];
		value->v.ipaddress[3] = udpoids[i].index.subs[3];
		break;

	  case LEAF_udpLocalPort:
		value->v.integer = udpoids[i].index.subs[4];
		break;

	}
	return (SNMP_ERR_NOERROR);
}