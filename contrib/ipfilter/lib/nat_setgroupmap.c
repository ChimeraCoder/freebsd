
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
 * Copyright (C) 2003 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 */
#if !defined(lint)
static const char rcsid[] = "@(#)$Id: nat_setgroupmap.c,v 1.1.4.1 2006/06/16 17:21:07 darrenr Exp $";
#endif

#include "ipf.h"

void nat_setgroupmap(n)
ipnat_t *n;
{
	if (n->in_outmsk == n->in_inmsk)
		n->in_ippip = 1;
	else if (n->in_flags & IPN_AUTOPORTMAP) {
		n->in_ippip = ~ntohl(n->in_inmsk);
		if (n->in_outmsk != 0xffffffff)
			n->in_ippip /= (~ntohl(n->in_outmsk) + 1);
		n->in_ippip++;
		if (n->in_ippip == 0)
			n->in_ippip = 1;
		n->in_ppip = USABLE_PORTS / n->in_ippip;
	} else {
		n->in_space = USABLE_PORTS * ~ntohl(n->in_outmsk);
		n->in_nip = 0;
		if (!(n->in_ppip = n->in_pmin))
			n->in_ppip = 1;
		n->in_ippip = USABLE_PORTS / n->in_ppip;
	}
}