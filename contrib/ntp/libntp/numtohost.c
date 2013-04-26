
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
/*
 * numtohost - convert network number to host name.
 */
#include "ntp_fp.h"
#include "ntp_stdlib.h"
#include "lib_strbuf.h"

#define	LOOPBACKNET	0x7f000000
#define	LOOPBACKHOST	0x7f000001
#define	LOOPBACKNETMASK	0xff000000

char *
numtohost(
	u_int32 netnum
	)
{
	char *bp;
	struct hostent *hp;

	/*
	 * This is really gross, but saves lots of hanging looking for
	 * hostnames for the radio clocks.  Don't bother looking up
	 * addresses on the loopback network except for the loopback
	 * host itself.
	 */
	if ((((ntohl(netnum) & LOOPBACKNETMASK) == LOOPBACKNET)
	     && (ntohl(netnum) != LOOPBACKHOST))
	    || ((hp = gethostbyaddr((char *)&netnum, sizeof netnum, AF_INET))
		== 0))
	    return numtoa(netnum);
	
	LIB_GETBUF(bp);
	
	bp[LIB_BUFLENGTH-1] = '\0';
	(void) strncpy(bp, hp->h_name, LIB_BUFLENGTH-1);
	return bp;
}