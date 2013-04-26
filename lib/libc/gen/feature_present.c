
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Returns true if the named feature is present in the currently
 * running kernel.  A feature's presence is indicated by an integer
 * sysctl node called kern.feature.<feature> that is non-zero.
 */
int
feature_present(const char *feature)
{
	char *mib;
	size_t len;
	int i;

	if (asprintf(&mib, "kern.features.%s", feature) < 0)
		return (0);
	len = sizeof(i);
	if (sysctlbyname(mib, &i, &len, NULL, 0) < 0) {
		free(mib);
		return (0);
	}
	free(mib);
	if (len != sizeof(i))
		return (0);
	return (i != 0);
}