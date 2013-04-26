
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
#include <sys/iconv.h>
#include <sys/sysctl.h>

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int
kiconv_lookupconv(const char *drvname)
{
	size_t size;

	if (sysctlbyname("kern.iconv.drvlist", NULL, &size, NULL, 0) == -1)
		return (errno);
	if (size > 0) {
		char *drivers, *drvp;

		drivers = malloc(size);
		if (drivers == NULL)
			return (ENOMEM);
		if (sysctlbyname("kern.iconv.drvlist", drivers, &size, NULL, 0) == -1) {
			free(drivers);
			return (errno);
		}
		for (drvp = drivers; *drvp != '\0'; drvp += strlen(drvp) + 1)
			if (strcmp(drvp, drvname) == 0) {
				free(drivers);
				return (0);
			}
	}
	return (ENOENT);
}

int
kiconv_lookupcs(const char *tocode, const char *fromcode)
{
	size_t i, size;
	struct iconv_cspair_info *csi, *csip;

	if (sysctlbyname("kern.iconv.cslist", NULL, &size, NULL, 0) == -1)
		return (errno);
	if (size > 0) {
		csi = malloc(size);
		if (csi == NULL)
			return (ENOMEM);
		if (sysctlbyname("kern.iconv.cslist", csi, &size, NULL, 0) == -1) {
			free(csi);
			return (errno);
		}
		for (i = 0, csip = csi; i < (size/sizeof(*csi)); i++, csip++){
			if (strcmp(csip->cs_to, tocode) == 0 &&
			    strcmp(csip->cs_from, fromcode) == 0) {
				free(csi);
				return (0);
			}
		}
	}
	return (ENOENT);
}