
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

/* $Id: inet_aton.c,v 1.6 2003-11-16 09:36:49 guy Exp $ */

#ifndef lint
static const char rcsid[] _U_ =
     "@(#) $Header: /tcpdump/master/tcpdump/missing/inet_aton.c,v 1.6 2003-11-16 09:36:49 guy Exp $";
#endif

#include <tcpdump-stdinc.h>

/* Minimal implementation of inet_aton.
 * Cannot distinguish between failure and a local broadcast address. */

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

int
inet_aton(const char *cp, struct in_addr *addr)
{
  addr->s_addr = inet_addr(cp);
  return (addr->s_addr == INADDR_NONE) ? 0 : 1;
}