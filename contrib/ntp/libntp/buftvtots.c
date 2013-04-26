
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
 * buftvtots - pull a Unix-format (struct timeval) time stamp out of
 *	       an octet stream and convert it to a l_fp time stamp.
 *	       This is useful when using the clock line discipline.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "ntp_fp.h"
#include "ntp_string.h"
#include "ntp_unixtime.h"

int
buftvtots(
	const char *bufp,
	l_fp *ts
	)
{
	struct timeval tv;

	/*
	 * copy to adhere to alignment restrictions
	 */
	memcpy(&tv, bufp, sizeof(tv));

	/*
	 * and use it
	 */
	ts->l_ui = tv.tv_sec + (u_long)JAN_1970;
	if (tv.tv_usec > 999999)
	    return 0;
	TVUTOTSF(tv.tv_usec, ts->l_uf);
	return 1;
}