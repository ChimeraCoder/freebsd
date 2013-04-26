
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
 * Copyright (C) 2000-2002 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: addipopt.c,v 1.7.4.1 2006/06/16 17:20:56 darrenr Exp $
 */

#include "ipf.h"


int addipopt(op, io, len, class)
char *op;
struct ipopt_names *io;
int len;
char *class;
{
	int olen = len;
	struct in_addr ipadr;
	u_short val;
	u_char lvl;
	char *s;

	if ((len + io->on_siz) > 48) {
		fprintf(stderr, "options too long\n");
		return 0;
	}
	len += io->on_siz;
	*op++ = io->on_value;
	if (io->on_siz > 1) {
		s = op;
		*op++ = io->on_siz;
		*op++ = IPOPT_MINOFF;

		if (class) {
			switch (io->on_value)
			{
			case IPOPT_SECURITY :
				lvl = seclevel(class);
				*(op - 1) = lvl;
				break;
			case IPOPT_LSRR :
			case IPOPT_SSRR :
				ipadr.s_addr = inet_addr(class);
				s[IPOPT_OLEN] = IPOPT_MINOFF - 1 + 4;
				bcopy((char *)&ipadr, op, sizeof(ipadr));
				break;
			case IPOPT_SATID :
				val = atoi(class);
				bcopy((char *)&val, op, 2);
				break;
			}
		}

		op += io->on_siz - 3;
		if (len & 3) {
			*op++ = IPOPT_NOP;
			len++;
		}
	}
	if (opts & OPT_DEBUG)
		fprintf(stderr, "bo: %s %d %#x: %d\n",
			io->on_name, io->on_value, io->on_bit, len);
	return len - olen;
}