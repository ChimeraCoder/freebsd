
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
 * numtoa - return asciized network numbers store in local array space
 */#include <stdio.h>

#include "ntp_fp.h"
#include "lib_strbuf.h"
#include "ntp_stdlib.h"

char *
numtoa(
	u_int32 num
	)
{
	register u_int32 netnum;
	register char *buf;

	netnum = ntohl(num);
	LIB_GETBUF(buf);
	(void) sprintf(buf, "%lu.%lu.%lu.%lu", ((u_long)netnum >> 24) & 0xff,
		       ((u_long)netnum >> 16) & 0xff, ((u_long)netnum >> 8) & 0xff,
		       (u_long)netnum & 0xff);
	return buf;
}