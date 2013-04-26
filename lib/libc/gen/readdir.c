
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)readdir.c	8.3 (Berkeley) 9/29/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "un-namespace.h"

#include "libc_private.h"
#include "gen-private.h"
#include "telldir.h"

/*
 * get next entry in a directory.
 */
struct dirent *
_readdir_unlocked(dirp, skip)
	DIR *dirp;
	int skip;
{
	struct dirent *dp;

	for (;;) {
		if (dirp->dd_loc >= dirp->dd_size) {
			if (dirp->dd_flags & __DTF_READALL)
				return (NULL);
			dirp->dd_loc = 0;
		}
		if (dirp->dd_loc == 0 && !(dirp->dd_flags & __DTF_READALL)) {
			dirp->dd_size = _getdirentries(dirp->dd_fd,
			    dirp->dd_buf, dirp->dd_len, &dirp->dd_seek);
			if (dirp->dd_size <= 0)
				return (NULL);
		}
		dp = (struct dirent *)(dirp->dd_buf + dirp->dd_loc);
		if ((long)dp & 03L)	/* bogus pointer check */
			return (NULL);
		if (dp->d_reclen <= 0 ||
		    dp->d_reclen > dirp->dd_len + 1 - dirp->dd_loc)
			return (NULL);
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_ino == 0 && skip)
			continue;
		if (dp->d_type == DT_WHT && (dirp->dd_flags & DTF_HIDEW))
			continue;
		return (dp);
	}
}

struct dirent *
readdir(dirp)
	DIR *dirp;
{
	struct dirent	*dp;

	if (__isthreaded) {
		_pthread_mutex_lock(&dirp->dd_lock);
		dp = _readdir_unlocked(dirp, 1);
		_pthread_mutex_unlock(&dirp->dd_lock);
	}
	else
		dp = _readdir_unlocked(dirp, 1);
	return (dp);
}

int
readdir_r(dirp, entry, result)
	DIR *dirp;
	struct dirent *entry;
	struct dirent **result;
{
	struct dirent *dp;
	int saved_errno;

	saved_errno = errno;
	errno = 0;
	if (__isthreaded) {
		_pthread_mutex_lock(&dirp->dd_lock);
		if ((dp = _readdir_unlocked(dirp, 1)) != NULL)
			memcpy(entry, dp, _GENERIC_DIRSIZ(dp));
		_pthread_mutex_unlock(&dirp->dd_lock);
	}
	else if ((dp = _readdir_unlocked(dirp, 1)) != NULL)
		memcpy(entry, dp, _GENERIC_DIRSIZ(dp));

	if (errno != 0) {
		if (dp == NULL)
			return (errno);
	} else
		errno = saved_errno;

	if (dp != NULL)
		*result = entry;
	else
		*result = NULL;

	return (0);
}