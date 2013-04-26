
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
#if 0
static char sccsid[] = "@(#)byteorder.c	8.1 (Berkeley) 6/6/93";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include "globals.h"

/*
 * Two routines to do the necessary byte swapping for timed protocol
 * messages. Protocol is defined in /usr/include/protocols/timed.h
 */
void
bytenetorder(struct tsp *ptr)
{
	ptr->tsp_seq = htons((u_short)ptr->tsp_seq);
	switch (ptr->tsp_type) {

	case TSP_SETTIME:
	case TSP_ADJTIME:
	case TSP_SETDATE:
	case TSP_SETDATEREQ:
		ptr->tsp_time.tv_sec = htonl((u_long)ptr->tsp_time.tv_sec);
		ptr->tsp_time.tv_usec = htonl((u_long)ptr->tsp_time.tv_usec);
		break;

	default:
		break;		/* nothing more needed */
	}
}

void
bytehostorder(struct tsp *ptr)
{
	ptr->tsp_seq = ntohs((u_short)ptr->tsp_seq);
	switch (ptr->tsp_type) {

	case TSP_SETTIME:
	case TSP_ADJTIME:
	case TSP_SETDATE:
	case TSP_SETDATEREQ:
		ptr->tsp_time.tv_sec = ntohl((u_long)ptr->tsp_time.tv_sec);
		ptr->tsp_time.tv_usec = ntohl((u_long)ptr->tsp_time.tv_usec);
		break;

	default:
		break;		/* nothing more needed */
	}
}