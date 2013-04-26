
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

#include "opt_ddb.h"

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifdef DDB
#include <ddb/ddb.h>
#endif

#include <net/if.h>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net/vnet.h>

#ifdef DDB
struct ifindex_entry {
	struct  ifnet *ife_ifnet;
};
VNET_DECLARE(struct ifindex_entry *, ifindex_table);
#define	V_ifindex_table		VNET(ifindex_table)

static void
if_show_ifnet(struct ifnet *ifp)
{

	if (ifp == NULL)
		return;
	db_printf("%s:\n", ifp->if_xname);
#define	IF_DB_PRINTF(f, e)	db_printf("   %s = " f "\n", #e, ifp->e);
	IF_DB_PRINTF("%s", if_dname);
	IF_DB_PRINTF("%d", if_dunit);
	IF_DB_PRINTF("%s", if_description);
	IF_DB_PRINTF("%u", if_index);
	IF_DB_PRINTF("%u", if_refcount);
	IF_DB_PRINTF("%d", if_index_reserved);
	IF_DB_PRINTF("%p", if_softc);
	IF_DB_PRINTF("%p", if_l2com);
	IF_DB_PRINTF("%p", if_vnet);
	IF_DB_PRINTF("%p", if_home_vnet);
	IF_DB_PRINTF("%p", if_addr);
	IF_DB_PRINTF("%p", if_llsoftc);
	IF_DB_PRINTF("%p", if_label);
	IF_DB_PRINTF("%u", if_pcount);
	IF_DB_PRINTF("0x%08x", if_flags);
	IF_DB_PRINTF("0x%08x", if_drv_flags);
	IF_DB_PRINTF("0x%08x", if_capabilities);
	IF_DB_PRINTF("0x%08x", if_capenable);
	IF_DB_PRINTF("%p", if_snd.ifq_head);
	IF_DB_PRINTF("%p", if_snd.ifq_tail);
	IF_DB_PRINTF("%d", if_snd.ifq_len);
	IF_DB_PRINTF("%d", if_snd.ifq_maxlen);
	IF_DB_PRINTF("%d", if_snd.ifq_drops);
	IF_DB_PRINTF("%p", if_snd.ifq_drv_head);
	IF_DB_PRINTF("%p", if_snd.ifq_drv_tail);
	IF_DB_PRINTF("%d", if_snd.ifq_drv_len);
	IF_DB_PRINTF("%d", if_snd.ifq_drv_maxlen);
	IF_DB_PRINTF("%d", if_snd.altq_type);
	IF_DB_PRINTF("%x", if_snd.altq_flags);
	IF_DB_PRINTF("%u", if_fib);
#undef IF_DB_PRINTF
}

DB_SHOW_COMMAND(ifnet, db_show_ifnet)
{

	if (!have_addr) {
		db_printf("usage: show ifnet <struct ifnet *>\n");
		return;
	}

	if_show_ifnet((struct ifnet *)addr);
}

DB_SHOW_ALL_COMMAND(ifnets, db_show_all_ifnets)
{
	VNET_ITERATOR_DECL(vnet_iter);
	struct ifnet *ifp;
	u_short idx;

	VNET_FOREACH(vnet_iter) {
		CURVNET_SET_QUIET(vnet_iter);
#ifdef VIMAGE
		db_printf("vnet=%p\n", curvnet);
#endif
		for (idx = 1; idx <= V_if_index; idx++) {
			ifp = V_ifindex_table[idx].ife_ifnet;
			if (ifp == NULL)
				continue;
			db_printf( "%20s ifp=%p\n", ifp->if_xname, ifp);
			if (db_pager_quit)
				break;
		}
		CURVNET_RESTORE();
	}
}
#endif