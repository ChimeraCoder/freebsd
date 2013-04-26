
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
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <string.h>

int	mkpath(char *);

/* Code taken directly from mkdir(1).

 * mkpath -- create directories.
 *	path     - path
 */
int
mkpath(char *path)
{
	struct stat sb;
	char *slash;
	int done = 0;

	slash = path;

	while (!done) {
		slash += strspn(slash, "/");
		slash += strcspn(slash, "/");

		done = (*slash == '\0');
		*slash = '\0';

		if (stat(path, &sb)) {
			if (errno != ENOENT || (mkdir(path, 0777) &&
			    errno != EEXIST)) {
				warn("%s", path);
				return (-1);
			}
		} else if (!S_ISDIR(sb.st_mode)) {
			warnx("%s: %s", path, strerror(ENOTDIR));
			return (-1);
		}

		*slash = '/';
	}

	return (0);
}