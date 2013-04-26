
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
 * Copyright (C) 2002-2004 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * Added redirect stuff and a variety of bug fixes. (mcn@EnGarde.com)
 */
#include "ipf.h"
#include "kmem.h"

#if !defined(lint)
static const char rcsid[] = "@(#)$Id: getnattype.c,v 1.3.2.2 2006/07/14 06:12:24 darrenr Exp $";
#endif


/*
 * Get a nat filter type given its kernel address.
 */
char *getnattype(nat, alive)
nat_t *nat;
int alive;
{
	static char unknownbuf[20];
	ipnat_t *ipn, ipnat;
	char *which;
	int type;

	if (!nat)
		return "???";
	if (alive) {
		type = nat->nat_redir;
	} else {
		ipn = nat->nat_ptr;
		if (kmemcpy((char *)&ipnat, (long)ipn, sizeof(ipnat)))
			return "!!!";
		type = ipnat.in_redir;
	}

	switch (type)
	{
	case NAT_MAP :
		which = "MAP";
		break;
	case NAT_MAPBLK :
		which = "MAP-BLOCK";
		break;
	case NAT_REDIRECT :
		which = "RDR";
		break;
	case NAT_BIMAP :
		which = "BIMAP";
		break;
	default :
		sprintf(unknownbuf, "unknown(%04x)", type & 0xffffffff);
		which = unknownbuf;
		break;
	}
	return which;
}