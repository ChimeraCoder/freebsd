
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

#include "telnetd.h"

RCSID("$Id$");

/*
 * get_slc_defaults
 *
 * Initialize the slc mapping table.
 */
void
get_slc_defaults(void)
{
    int i;

    init_termbuf();

    for (i = 1; i <= NSLC; i++) {
	slctab[i].defset.flag =
	    spcset(i, &slctab[i].defset.val, &slctab[i].sptr);
	slctab[i].current.flag = SLC_NOSUPPORT;
	slctab[i].current.val = 0;
    }

}