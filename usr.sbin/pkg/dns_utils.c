
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

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <resolv.h>

#include "dns_utils.h"

typedef union {
	HEADER hdr;
	unsigned char buf[1024];
} dns_query;

struct dns_srvinfo *
dns_getsrvinfo(const char *zone)
{
	struct dns_srvinfo **res, *first;
	unsigned char *end, *p;
	char host[MAXHOSTNAMELEN];
	dns_query q;
	int len, qdcount, ancount, n, i;
	unsigned int type, class, ttl, priority, weight, port;

	if ((len = res_query(zone, C_IN, T_SRV, q.buf, sizeof(q.buf))) == -1 ||
	    len < (int)sizeof(HEADER))
		return (NULL);

	qdcount = ntohs(q.hdr.qdcount);
	ancount = ntohs(q.hdr.ancount);

	end = q.buf + len;
	p = q.buf + sizeof(HEADER);

	while(qdcount > 0 && p < end) {
		qdcount--;
		if((len = dn_expand(q.buf, end, p, host, MAXHOSTNAMELEN)) < 0)
			return (NULL);
		p += len + NS_QFIXEDSZ;
	}

	res = calloc(ancount, sizeof(struct dns_srvinfo));
	if (res == NULL)
		return (NULL);

	n = 0;
	while (ancount > 0 && p < end) {
		ancount--;
		len = dn_expand(q.buf, end, p, host, MAXHOSTNAMELEN);
		if (len < 0) {
			for (i = 0; i < n; i++)
				free(res[i]);
			free(res);
			return NULL;
		}

		p += len;

		NS_GET16(type, p);
		NS_GET16(class, p);
		NS_GET32(ttl, p);
		NS_GET16(len, p);

		if (type != T_SRV) {
			p += len;
			continue;
		}

		NS_GET16(priority, p);
		NS_GET16(weight, p);
		NS_GET16(port, p);

		len = dn_expand(q.buf, end, p, host, MAXHOSTNAMELEN);
		if (len < 0) {
			for (i = 0; i < n; i++)
				free(res[i]);
			free(res);
			return (NULL);
		}

		res[n] = malloc(sizeof(struct dns_srvinfo));
		if (res[n] == NULL) {
			for (i = 0; i < n; i++)
				free(res[i]);
			free(res);
			return (NULL);
		}
		res[n]->type = type;
		res[n]->class = class;
		res[n]->ttl = ttl;
		res[n]->priority = priority;
		res[n]->weight = weight;
		res[n]->port = port;
		res[n]->next = NULL;
		strlcpy(res[n]->host, host, MAXHOSTNAMELEN);

		p += len;
		n++;
	}

	for (i = 0; i < n - 1; i++)
		res[i]->next = res[i + 1];

	first = res[0];
	free(res);

	return (first);
}