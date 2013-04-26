
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
 * $Id: buildopts.c,v 1.6.4.1 2006/06/16 17:20:56 darrenr Exp $
 */

#include "ipf.h"


u_32_t buildopts(cp, op, len)
char *cp, *op;
int len;
{
	struct ipopt_names *io;
	u_32_t msk = 0;
	char *s, *t;
	int inc;

	for (s = strtok(cp, ","); s; s = strtok(NULL, ",")) {
		if ((t = strchr(s, '=')))
			*t++ = '\0';
		for (io = ionames; io->on_name; io++) {
			if (strcasecmp(s, io->on_name) || (msk & io->on_bit))
				continue;
			if ((inc = addipopt(op, io, len, t))) {
				op += inc;
				len += inc;
			}
			msk |= io->on_bit;
			break;
		}
		if (!io->on_name) {
			fprintf(stderr, "unknown IP option name %s\n", s);
			return 0;
		}
	}
	*op++ = IPOPT_EOL;
	len++;
	return len;
}