
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

#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/zone.h>

zoneid_t
getzoneid(void)
{
	size_t size;
	int jailid;

	/* Information that we are in jail or not is enough for our needs. */
	size = sizeof(jailid);
	if (sysctlbyname("security.jail.jailed", &jailid, &size, NULL, 0) == -1)
		assert(!"No security.jail.jailed sysctl!");
	return ((zoneid_t)jailid);
}