
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

iphtent_t *printhashnode(iph, ipep, copyfunc, opts)
iphtable_t *iph;
iphtent_t *ipep;
copyfunc_t copyfunc;
int opts;
{
	iphtent_t ipe;

	if ((*copyfunc)(ipep, &ipe, sizeof(ipe)))
		return NULL;

	ipe.ipe_addr.in4_addr = htonl(ipe.ipe_addr.in4_addr);
	ipe.ipe_mask.in4_addr = htonl(ipe.ipe_mask.in4_addr);

	if ((opts & OPT_DEBUG) != 0) {
		PRINTF("\tAddress: %s",
			inet_ntoa(ipe.ipe_addr.in4));
		printmask((u_32_t *)&ipe.ipe_mask.in4_addr);
		PRINTF("\tRef. Count: %d\tGroup: %s\n", ipe.ipe_ref,
			ipe.ipe_group);
	} else {
		putchar(' ');
		printip((u_32_t *)&ipe.ipe_addr.in4_addr);
		printmask((u_32_t *)&ipe.ipe_mask.in4_addr);
		if (ipe.ipe_value != 0) {
			switch (iph->iph_type & ~IPHASH_ANON)
			{
			case IPHASH_GROUPMAP :
				if (strncmp(ipe.ipe_group, iph->iph_name,
					    FR_GROUPLEN))
					PRINTF(", group = %s", ipe.ipe_group);
				break;
			}
		}
		putchar(';');
	}

	ipep = ipe.ipe_next;
	return ipep;
}