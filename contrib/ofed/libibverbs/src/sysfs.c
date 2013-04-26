
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <sys/sysctl.h>

#include "ibverbs.h"

static char *sysfs_path;

const char *ibv_get_sysfs_path(void)
{
	char *env = NULL;

	if (sysfs_path)
		return sysfs_path;

	/*
	 * Only follow use path passed in through the calling user's
	 * environment if we're not running SUID.
	 */
	if (getuid() == geteuid())
		env = getenv("SYSFS_PATH");

	if (env) {
		int len;

		sysfs_path = strndup(env, IBV_SYSFS_PATH_MAX);
		len = strlen(sysfs_path);
		while (len > 0 && sysfs_path[len - 1] == '/') {
			--len;
			sysfs_path[len] = '\0';
		}
	} else
		sysfs_path = "/sys";

	return sysfs_path;
}

int ibv_read_sysfs_file(const char *dir, const char *file,
			char *buf, size_t size)
{
	char *path, *s;
	int fd;
	size_t len;

	if (asprintf(&path, "%s/%s", dir, file) < 0)
		return -1;

	for (s = &path[0]; *s != '\0'; s++)
		if (*s == '/')
			*s = '.';

        len = size;
        if (sysctlbyname(&path[1], buf, &len, NULL, 0) == -1)
		return -1;

	free(path);

	if (len > 0 && buf[len - 1] == '\n')
		buf[--len] = '\0';

	return len;
}