
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
 * $Id: printtunable.c,v 1.1.4.1 2006/06/16 17:21:15 darrenr Exp $ 
 */     

#include "ipf.h"

void printtunable(tup)
ipftune_t *tup;
{
	printf("%s\tmin %#lx\tmax %#lx\tcurrent ",
		tup->ipft_name, tup->ipft_min, tup->ipft_max);
	if (tup->ipft_sz == sizeof(u_long))
		printf("%lu\n", tup->ipft_vlong);
	else if (tup->ipft_sz == sizeof(u_int))
		printf("%u\n", tup->ipft_vint);
	else if (tup->ipft_sz == sizeof(u_short))
		printf("%hu\n", tup->ipft_vshort);
	else if (tup->ipft_sz == sizeof(u_char))
		printf("%u\n", (u_int)tup->ipft_vchar);
	else {
		printf("sz = %d\n", tup->ipft_sz);
	}
}