
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
 *
 * $Id: remove_hash.c,v 1.1.4.1 2006/06/16 17:21:16 darrenr Exp $
 */

#include <fcntl.h>
#include <sys/ioctl.h>
#include "ipf.h"
#include "netinet/ip_lookup.h"
#include "netinet/ip_htable.h"

static int hashfd = -1;


int remove_hash(iphp, iocfunc)
iphtable_t *iphp;
ioctlfunc_t iocfunc;
{
	iplookupop_t op;
	iphtable_t iph;

	if ((hashfd == -1) && ((opts & OPT_DONOTHING) == 0))
		hashfd = open(IPLOOKUP_NAME, O_RDWR);
	if ((hashfd == -1) && ((opts & OPT_DONOTHING) == 0))
		return -1;

	op.iplo_type = IPLT_HASH;
	op.iplo_unit = iphp->iph_unit;
	strncpy(op.iplo_name, iphp->iph_name, sizeof(op.iplo_name));
	if (*op.iplo_name == '\0')
		op.iplo_arg = IPHASH_ANON;
	op.iplo_size = sizeof(iph);
	op.iplo_struct = &iph;

	bzero((char *)&iph, sizeof(iph));
	iph.iph_unit = iphp->iph_unit;
	iph.iph_type = iphp->iph_type;
	strncpy(iph.iph_name, iphp->iph_name, sizeof(iph.iph_name));
	iph.iph_flags = iphp->iph_flags;

	if ((*iocfunc)(hashfd, SIOCLOOKUPDELTABLE, &op))
		if ((opts & OPT_DONOTHING) == 0) {
			perror("remove_hash:SIOCLOOKUPDELTABLE");
			return -1;
		}

	return 0;
}