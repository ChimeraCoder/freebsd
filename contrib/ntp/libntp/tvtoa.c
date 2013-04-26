
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
 * tvtoa - return an asciized representation of a struct timeval
 */
#include "lib_strbuf.h"

#if defined(VMS)
# include "ntp_fp.h"
#endif /* VMS */
#include "ntp_stdlib.h"
#include "ntp_unixtime.h"

#include <stdio.h>

char *
tvtoa(
	const struct timeval *tv
	)
{
	register char *buf;
	register u_long sec;
	register u_long usec;
	register int isneg;

	if (tv->tv_sec < 0 || tv->tv_usec < 0) {
		sec = -tv->tv_sec;
		usec = -tv->tv_usec;
		isneg = 1;
	} else {
		sec = tv->tv_sec;
		usec = tv->tv_usec;
		isneg = 0;
	}

	LIB_GETBUF(buf);

	(void) sprintf(buf, "%s%lu.%06lu", (isneg?"-":""), sec, usec);
	return buf;
}