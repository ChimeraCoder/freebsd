
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

/*
 * I din't use "namespace.h" here because none of the relevant utilities
 * are threaded, so I'm not concerned about cancellation points or other
 * niceties.
 */
#include <sys/limits.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	_PATH_UTIL_COMPAT	"/etc/compat-FreeBSD-4-util"
#define	_ENV_UTIL_COMPAT	"_COMPAT_FreeBSD_4"

int
check_utility_compat(const char *utility)
{
	char buf[PATH_MAX];
	char *p, *bp;
	int len;

	if ((p = getenv(_ENV_UTIL_COMPAT)) != NULL) {
		strlcpy(buf, p, sizeof buf);
	} else {
		if ((len = readlink(_PATH_UTIL_COMPAT, buf, sizeof(buf) - 1)) < 0)
			return 0;
		buf[len] = '\0';
	}
	if (buf[0] == '\0')
		return 1;

	bp = buf;
	while ((p = strsep(&bp, ",")) != NULL) {
		if (strcmp(p, utility) == 0)
			return 1;
	}
	return 0;
}