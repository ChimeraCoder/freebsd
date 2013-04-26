
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

#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <string.h>
#include "libgeom.h"

char *
geom_getxml(void)
{
	char *p;
	size_t l = 0;
	int mib[3];
	size_t sizep;

	sizep = sizeof(mib) / sizeof(*mib);
	if (sysctlnametomib("kern.geom.confxml", mib, &sizep) != 0)
		return (NULL);
	if (sysctl(mib, sizep, NULL, &l, NULL, 0) != 0)
		return (NULL);
	l += 4096;
	p = malloc(l);
	if (p == NULL)
		return (NULL);
	if (sysctl(mib, sizep, p, &l, NULL, 0) != 0) {
		free(p);
		return (NULL);
	}
	return (reallocf(p, strlen(p) + 1));
}