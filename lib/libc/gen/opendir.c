
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
static char sccsid[] = "@(#)opendir.c	8.8 (Berkeley) 5/1/95";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"

#include "gen-private.h"
#include "telldir.h"

static DIR * __opendir_common(int, int);

/*
 * Open a directory.
 */
DIR *
opendir(const char *name)
{

	return (__opendir2(name, DTF_HIDEW|DTF_NODUP));
}

/*
 * Open a directory with existing file descriptor.
 */
DIR *
fdopendir(int fd)
{
	struct stat statb;

	/* Check that fd is associated with a directory. */
	if (_fstat(fd, &statb) != 0)
		return (NULL);
	if (!S_ISDIR(statb.st_mode)) {
		errno = ENOTDIR;
		return (NULL);
	}
	if (_fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
		return (NULL);
	return (__opendir_common(fd, DTF_HIDEW|DTF_NODUP));
}

DIR *
__opendir2(const char *name, int flags)
{
	int fd;
	DIR *dir;
	int saved_errno;

	if ((fd = _open(name,
	    O_RDONLY | O_NONBLOCK | O_DIRECTORY | O_CLOEXEC)) == -1)
		return (NULL);

	dir = __opendir_common(fd, flags);
	if (dir == NULL) {
		saved_errno = errno;
		_close(fd);
		errno = saved_errno;
	}
	return (dir);
}

static int
opendir_compar(const void *p1, const void *p2)
{

	return (strcmp((*(const struct dirent **)p1)->d_name,
	    (*(const struct dirent **)p2)->d_name));
}

/*
 * Common routine for opendir(3), __opendir2(3) and fdopendir(3).
 */
static DIR *
__opendir_common(int fd, int flags)
{
	DIR *dirp;
	int incr;
	int saved_errno;
	int unionstack;
	int fd2;

	fd2 = -1;

	if ((dirp = malloc(sizeof(DIR) + sizeof(struct _telldir))) == NULL)
		return (NULL);

	dirp->dd_td = (struct _telldir *)((char *)dirp + sizeof(DIR));
	LIST_INIT(&dirp->dd_td->td_locq);
	dirp->dd_td->td_loccnt = 0;

	/*
	 * Use the system page size if that is a multiple of DIRBLKSIZ.
	 * Hopefully this can be a big win someday by allowing page
	 * trades to user space to be done by _getdirentries().
	 */
	incr = getpagesize();
	if ((incr % DIRBLKSIZ) != 0) 
		incr = DIRBLKSIZ;

	/*
	 * Determine whether this directory is the top of a union stack.
	 */
	if (flags & DTF_NODUP) {
		struct statfs sfb;

		if (_fstatfs(fd, &sfb) < 0)
			goto fail;
		unionstack = !strcmp(sfb.f_fstypename, "unionfs")
		    || (sfb.f_flags & MNT_UNION);
	} else {
		unionstack = 0;
	}

	if (unionstack) {
		int len = 0;
		int space = 0;
		char *buf = 0;
		char *ddptr = 0;
		char *ddeptr;
		int n;
		struct dirent **dpv;

		/*
		 * The strategy here is to read all the directory
		 * entries into a buffer, sort the buffer, and
		 * remove duplicate entries by setting the inode
		 * number to zero.
		 *
		 * We reopen the directory because _getdirentries()
		 * on a MNT_UNION mount modifies the open directory,
		 * making it refer to the lower directory after the
		 * upper directory's entries are exhausted.
		 * This would otherwise break software that uses
		 * the directory descriptor for fchdir or *at
		 * functions, such as fts.c.
		 */
		if ((fd2 = _openat(fd, ".", O_RDONLY | O_CLOEXEC)) == -1) {
			saved_errno = errno;
			free(buf);
			free(dirp);
			errno = saved_errno;
			return (NULL);
		}

		do {
			/*
			 * Always make at least DIRBLKSIZ bytes
			 * available to _getdirentries
			 */
			if (space < DIRBLKSIZ) {
				space += incr;
				len += incr;
				buf = reallocf(buf, len);
				if (buf == NULL)
					goto fail;
				ddptr = buf + (len - space);
			}

			n = _getdirentries(fd2, ddptr, space, &dirp->dd_seek);
			if (n > 0) {
				ddptr += n;
				space -= n;
			}
		} while (n > 0);

		ddeptr = ddptr;
		flags |= __DTF_READALL;

		_close(fd2);
		fd2 = -1;

		/*
		 * There is now a buffer full of (possibly) duplicate
		 * names.
		 */
		dirp->dd_buf = buf;

		/*
		 * Go round this loop twice...
		 *
		 * Scan through the buffer, counting entries.
		 * On the second pass, save pointers to each one.
		 * Then sort the pointers and remove duplicate names.
		 */
		for (dpv = 0;;) {
			n = 0;
			ddptr = buf;
			while (ddptr < ddeptr) {
				struct dirent *dp;

				dp = (struct dirent *) ddptr;
				if ((long)dp & 03L)
					break;
				if ((dp->d_reclen <= 0) ||
				    (dp->d_reclen > (ddeptr + 1 - ddptr)))
					break;
				ddptr += dp->d_reclen;
				if (dp->d_fileno) {
					if (dpv)
						dpv[n] = dp;
					n++;
				}
			}

			if (dpv) {
				struct dirent *xp;

				/*
				 * This sort must be stable.
				 */
				mergesort(dpv, n, sizeof(*dpv),
				    opendir_compar);

				dpv[n] = NULL;
				xp = NULL;

				/*
				 * Scan through the buffer in sort order,
				 * zapping the inode number of any
				 * duplicate names.
				 */
				for (n = 0; dpv[n]; n++) {
					struct dirent *dp = dpv[n];

					if ((xp == NULL) ||
					    strcmp(dp->d_name, xp->d_name)) {
						xp = dp;
					} else {
						dp->d_fileno = 0;
					}
					if (dp->d_type == DT_WHT &&
					    (flags & DTF_HIDEW))
						dp->d_fileno = 0;
				}

				free(dpv);
				break;
			} else {
				dpv = malloc((n+1) * sizeof(struct dirent *));
				if (dpv == NULL)
					break;
			}
		}

		dirp->dd_len = len;
		dirp->dd_size = ddptr - dirp->dd_buf;
	} else {
		dirp->dd_len = incr;
		dirp->dd_size = 0;
		dirp->dd_buf = malloc(dirp->dd_len);
		if (dirp->dd_buf == NULL)
			goto fail;
		dirp->dd_seek = 0;
	}

	dirp->dd_loc = 0;
	dirp->dd_fd = fd;
	dirp->dd_flags = flags;
	dirp->dd_lock = NULL;

	/*
	 * Set up seek point for rewinddir.
	 */
	dirp->dd_rewind = telldir(dirp);

	return (dirp);

fail:
	saved_errno = errno;
	if (fd2 != -1)
		_close(fd2);
	free(dirp);
	errno = saved_errno;
	return (NULL);
}