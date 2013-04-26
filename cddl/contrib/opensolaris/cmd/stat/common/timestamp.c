
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

#include "statcommon.h"

#include <langinfo.h>

/*
 * Print timestamp as decimal reprentation of time_t value (-T u was specified)
 * or in date(1) format (-T d was specified).
 */
void
print_timestamp(uint_t timestamp_fmt)
{
	time_t t = time(NULL);

	if (timestamp_fmt == UDATE) {
		(void) printf("%ld\n", t);
	} else if (timestamp_fmt == DDATE) {
		char dstr[64];
		int len;

		len = strftime(dstr, sizeof (dstr), "%+", localtime(&t));
		if (len > 0)
			(void) printf("%s\n", dstr);
	}
}