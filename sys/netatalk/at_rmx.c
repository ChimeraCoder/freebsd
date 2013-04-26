
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

/* This code generates debugging traces to the radix code. */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/socket.h>

#include <net/route.h>

int	at_inithead(void **head, int off);

#if 0
#define	HEXBUF_LEN	256

static const char *
prsockaddr(void *v, char *hexbuf)
{
	char *bp = &hexbuf[0];
	u_char *cp = v;

	if (v != NULL) {
		int len = *cp;
		u_char *cplim = cp + len;

		/* return: "(len) hexdump" */

		bp += sprintf(bp, "(%d)", len);
		for (cp++; cp < cplim && bp < hexbuf + (HEXBUF_LEN - 4);
		    cp++) {
			*bp++ = "0123456789abcdef"[*cp / 16];
			*bp++ = "0123456789abcdef"[*cp % 16];
		}
	} else
		bp+= sprintf(bp, "null");
	*bp = '\0';
	return (hexbuf);
}
#endif

static struct radix_node *
at_addroute(void *v_arg, void *n_arg, struct radix_node_head *head,
	    struct radix_node *treenodes)
{

	return (rn_addroute(v_arg, n_arg, head, treenodes));
}

static struct radix_node *
at_matroute(void *v_arg, struct radix_node_head *head)
{

	return (rn_match(v_arg, head));
}

static struct radix_node *
at_lookup(void *v_arg, void *m_arg, struct radix_node_head *head)
{

	return (rn_lookup(v_arg, m_arg, head));
}

static struct radix_node *
at_delroute(void *v_arg, void *netmask_arg, struct radix_node_head *head)
{

	return (rn_delete(v_arg, netmask_arg, head));
}

/*
 * Initialize our routing tree with debugging hooks.
 */
int
at_inithead(void **head, int off)
{
	struct radix_node_head *rnh;

	if (!rn_inithead(head, off))
		return (0);

	rnh = *head;
	rnh->rnh_addaddr = at_addroute;
	rnh->rnh_deladdr = at_delroute;
	rnh->rnh_matchaddr = at_matroute;
	rnh->rnh_lookup = at_lookup;
	return (1);
}