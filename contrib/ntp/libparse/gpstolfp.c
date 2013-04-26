
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
#include "ntp_fp.h"

#define GPSORIGIN	ULONG_CONST(2524953600)	/* NTP origin - GPS origin in seconds */
#define SECSPERWEEK	(unsigned)(604800)	/* seconds per week - GPS tells us about weeks */
#define GPSWRAP		990	/* assume week count less than this in the previous epoch */

void
gpstolfp(
	 int weeks,
	 int days,
	 unsigned long  seconds,
	 l_fp * lfp
	 )
{
  if (weeks < GPSWRAP)
    {
      weeks += 1024;
    }

  lfp->l_ui = weeks * SECSPERWEEK + days * 86400 + seconds + GPSORIGIN; /* convert to NTP time */
  lfp->l_uf = 0;
}

/*
 * History:
 *
 * gpstolfp.c,v
 * Revision 4.8  2005/04/16 17:32:10  kardel
 * update copyright
 *
 * Revision 4.7  2004/11/14 15:29:41  kardel
 * support PPSAPI, upgrade Copyright to Berkeley style
 *
 * Revision 4.3  1999/02/28 11:42:44  kardel
 * (GPSWRAP): update GPS rollover to 990 weeks
 *
 * Revision 4.2  1998/07/11 10:05:25  kardel
 * Release 4.0.73d reconcilation
 *
 * Revision 4.1  1998/06/28 16:47:15  kardel
 * added gpstolfp() function
 */