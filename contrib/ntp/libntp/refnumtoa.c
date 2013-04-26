
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
 * refnumtoa - return asciized refclock addresses stored in local array space
 */#include <stdio.h>

#include "ntp_fp.h"
#include "lib_strbuf.h"
#include "ntp_stdlib.h"

char *
refnumtoa(
	struct sockaddr_storage* num
	)
{
	register u_int32 netnum;
	register char *buf;
	register const char *rclock;

	LIB_GETBUF(buf);

	if(num->ss_family == AF_INET) {
		netnum = ntohl(((struct sockaddr_in*)num)->sin_addr.s_addr);
		rclock = clockname((int)((u_long)netnum >> 8) & 0xff);

		if (rclock != NULL)
		    (void)sprintf(buf, "%s(%lu)", rclock, (u_long)netnum & 0xff);
		else
	    	(void)sprintf(buf, "REFCLK(%lu,%lu)",
				  ((u_long)netnum >> 8) & 0xff, (u_long)netnum & 0xff);

	}
	else {
		(void)sprintf(buf, "refclock address type not implemented yet, use IPv4 refclock address.");
	}
	return buf;
}