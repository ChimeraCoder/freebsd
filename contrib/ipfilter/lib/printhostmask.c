
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
 * Copyright (C) 2000-2005 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: printhostmask.c,v 1.8.4.1 2006/06/16 17:21:12 darrenr Exp $
 */

#include "ipf.h"


void	printhostmask(v, addr, mask)
int	v;
u_32_t	*addr, *mask;
{
#ifdef  USE_INET6
	char ipbuf[64];
#else
	struct in_addr ipa;
#endif

	if (!*addr && !*mask)
		printf("any");
	else {
#ifdef  USE_INET6
		void *ptr = addr;
		int af;

		if (v == 4) {
			ptr = addr;
			af = AF_INET;
		} else if (v == 6) {
			ptr = addr;
			af = AF_INET6;
		} else
			af = 0;
		printf("%s", inet_ntop(af, ptr, ipbuf, sizeof(ipbuf)));
#else
		ipa.s_addr = *addr;
		printf("%s", inet_ntoa(ipa));
#endif
		printmask(mask);
	}
}