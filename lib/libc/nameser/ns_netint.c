
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

#ifndef lint
static const char rcsid[] = "$Id: ns_netint.c,v 1.2.18.1 2005/04/27 05:01:08 sra Exp $";
#endif

/* Import. */

#include "port_before.h"

#include <arpa/nameser.h>

#include "port_after.h"

/* Public. */

u_int
ns_get16(const u_char *src) {
	u_int dst;

	NS_GET16(dst, src);
	return (dst);
}

u_long
ns_get32(const u_char *src) {
	u_long dst;

	NS_GET32(dst, src);
	return (dst);
}

void
ns_put16(u_int src, u_char *dst) {
	NS_PUT16(src, dst);
}

void
ns_put32(u_long src, u_char *dst) {
	NS_PUT32(src, dst);
}

/*! \file */