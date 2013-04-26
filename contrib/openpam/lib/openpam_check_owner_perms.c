
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * OpenPAM internal
 *
 * Verify that the file or directory referenced by the given descriptor is
 * owned by either root or the arbitrator and that it is not writable by
 * group or other.
 */

int
openpam_check_desc_owner_perms(const char *name, int fd)
{
	uid_t root, arbitrator;
	struct stat sb;
	int serrno;

	root = 0;
	arbitrator = geteuid();
	if (fstat(fd, &sb) != 0) {
		serrno = errno;
		openpam_log(PAM_LOG_ERROR, "%s: %m", name);
		errno = serrno;
		return (-1);
	}
	if (!S_ISREG(sb.st_mode)) {
		openpam_log(PAM_LOG_ERROR,
		    "%s: not a regular file", name);
		errno = EINVAL;
		return (-1);
	}
	if ((sb.st_uid != root && sb.st_uid != arbitrator) ||
	    (sb.st_mode & (S_IWGRP|S_IWOTH)) != 0) {
		openpam_log(PAM_LOG_ERROR,
		    "%s: insecure ownership or permissions", name);
		errno = EPERM;
		return (-1);
	}
	return (0);
}

/*
 * OpenPAM internal
 *
 * Verify that a file or directory and all components of the path leading
 * up to it are owned by either root or the arbitrator and that they are
 * not writable by group or other.
 *
 * Note that openpam_check_desc_owner_perms() should be used instead if
 * possible to avoid a race between the ownership / permission check and
 * the actual open().
 */

int
openpam_check_path_owner_perms(const char *path)
{
	uid_t root, arbitrator;
	char pathbuf[PATH_MAX];
	struct stat sb;
	int len, serrno, tip;

	tip = 1;
	root = 0;
	arbitrator = geteuid();
	if (realpath(path, pathbuf) == NULL)
		return (-1);
	len = strlen(pathbuf);
	while (len > 0) {
		if (stat(pathbuf, &sb) != 0) {
			if (errno != ENOENT) {
				serrno = errno;
				openpam_log(PAM_LOG_ERROR, "%s: %m", pathbuf);
				errno = serrno;
			}
			return (-1);
		}
		if (tip && !S_ISREG(sb.st_mode)) {
			openpam_log(PAM_LOG_ERROR,
			    "%s: not a regular file", pathbuf);
			errno = EINVAL;
			return (-1);
		}
		if ((sb.st_uid != root && sb.st_uid != arbitrator) ||
		    (sb.st_mode & (S_IWGRP|S_IWOTH)) != 0) {
			openpam_log(PAM_LOG_ERROR,
			    "%s: insecure ownership or permissions", pathbuf);
			errno = EPERM;
			return (-1);
		}
		while (--len > 0 && pathbuf[len] != '/')
			pathbuf[len] = '\0';
		tip = 0;
	}
	return (0);
}

/*
 * NOPARSE
 */