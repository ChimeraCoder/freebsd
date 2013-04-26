
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
 *
 * $Id: printip.c,v 1.3.4.1 2006/06/16 17:21:12 darrenr Exp $
 */

#include "ipf.h"


void	printip(addr)
u_32_t	*addr;
{
	struct in_addr ipa;

	ipa.s_addr = *addr;
	if (ntohl(ipa.s_addr) < 256)
		printf("%lu", (u_long)ntohl(ipa.s_addr));
	else
		printf("%s", inet_ntoa(ipa));
}