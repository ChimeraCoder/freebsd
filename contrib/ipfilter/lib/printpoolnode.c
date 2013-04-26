
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
/*	$FreeBSD$	*/
/*
 * Copyright (C) 2002-2005 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 */

#include "ipf.h"

#define	PRINTF	(void)printf
#define	FPRINTF	(void)fprintf

ip_pool_node_t *printpoolnode(np, opts)
ip_pool_node_t *np;
int opts;
{

	if ((opts & OPT_DEBUG) == 0) {
		putchar(' ');
		if (np->ipn_info == 1)
			PRINTF("! ");
		printip((u_32_t *)&np->ipn_addr.adf_addr.in4);
		printmask((u_32_t *)&np->ipn_mask.adf_addr);
	} else {
		PRINTF("\tAddress: %s%s", np->ipn_info ? "! " : "",
			inet_ntoa(np->ipn_addr.adf_addr.in4));
		printmask((u_32_t *)&np->ipn_mask.adf_addr);
		PRINTF("\t\tHits %lu\tName %s\tRef %d\n",
			np->ipn_hits, np->ipn_name, np->ipn_ref);
	}
	return np->ipn_next;
}