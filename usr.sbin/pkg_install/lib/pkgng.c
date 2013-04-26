
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

#include "lib.h"
#include <err.h>

static const char message[] = "You appear to be using the newer pkg(1) tool on \
this system for package management, rather than the legacy package \
management tools (pkg_*).  The legacy tools should no longer be used on \
this system.";

void warnpkgng(void)
{
	char pkgngpath[MAXPATHLEN + 1];
	char *pkgngdir;
	char *dontwarn;
	int rc;

	dontwarn = getenv("PKG_OLD_NOWARN");
	if (dontwarn != NULL)
		return;
	pkgngdir = getenv("PKG_DBDIR");
	if (pkgngdir == NULL)
		pkgngdir = "/var/db/pkg";

	rc = snprintf(pkgngpath, sizeof(pkgngpath), "%s/local.sqlite", pkgngdir);
	if ((size_t)rc >= sizeof(pkgngpath)) {
		warnx("path too long: %s/local.sqlite", pkgngdir);
		return;
	}

	if (access(pkgngpath, F_OK) == 0)
		warnx(message);
}