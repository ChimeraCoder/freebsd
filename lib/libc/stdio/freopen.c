
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
static char sccsid[] = "@(#)freopen.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"

/*
 * Re-direct an existing, open (probably) file to some other file.
 * ANSI is written such that the original file gets closed if at
 * all possible, no matter what.
 */
FILE *
freopen(const char * __restrict file, const char * __restrict mode,
    FILE * __restrict fp)
{
	int f;
	int dflags, flags, isopen, oflags, sverrno, wantfd;

	if ((flags = __sflags(mode, &oflags)) == 0) {
		sverrno = errno;
		(void) fclose(fp);
		errno = sverrno;
		return (NULL);
	}

	FLOCKFILE(fp);

	if (!__sdidinit)
		__sinit();

	/*
	 * If the filename is a NULL pointer, the caller is asking us to
	 * re-open the same file with a different mode. We allow this only
	 * if the modes are compatible.
	 */
	if (file == NULL) {
		/* See comment below regarding freopen() of closed files. */
		if (fp->_flags == 0) {
			FUNLOCKFILE(fp);
			errno = EINVAL;
			return (NULL);
		}
		if ((dflags = _fcntl(fp->_file, F_GETFL)) < 0) {
			sverrno = errno;
			fclose(fp);
			FUNLOCKFILE(fp);
			errno = sverrno;
			return (NULL);
		}
		if ((dflags & O_ACCMODE) != O_RDWR && (dflags & O_ACCMODE) !=
		    (oflags & O_ACCMODE)) {
			fclose(fp);
			FUNLOCKFILE(fp);
			errno = EINVAL;
			return (NULL);
		}
		if (fp->_flags & __SWR)
			(void) __sflush(fp);
		if ((oflags ^ dflags) & O_APPEND) {
			dflags &= ~O_APPEND;
			dflags |= oflags & O_APPEND;
			if (_fcntl(fp->_file, F_SETFL, dflags) < 0) {
				sverrno = errno;
				fclose(fp);
				FUNLOCKFILE(fp);
				errno = sverrno;
				return (NULL);
			}
		}
		if (oflags & O_TRUNC)
			(void) ftruncate(fp->_file, (off_t)0);
		if (!(oflags & O_APPEND))
			(void) _sseek(fp, (fpos_t)0, SEEK_SET);
		if (oflags & O_CLOEXEC)
			(void) _fcntl(fp->_file, F_SETFD, FD_CLOEXEC);
		f = fp->_file;
		isopen = 0;
		wantfd = -1;
		goto finish;
	}

	/*
	 * There are actually programs that depend on being able to "freopen"
	 * descriptors that weren't originally open.  Keep this from breaking.
	 * Remember whether the stream was open to begin with, and which file
	 * descriptor (if any) was associated with it.  If it was attached to
	 * a descriptor, defer closing it; freopen("/dev/stdin", "r", stdin)
	 * should work.  This is unnecessary if it was not a Unix file.
	 */
	if (fp->_flags == 0) {
		fp->_flags = __SEOF;	/* hold on to it */
		isopen = 0;
		wantfd = -1;
	} else {
		/* flush the stream; ANSI doesn't require this. */
		if (fp->_flags & __SWR)
			(void) __sflush(fp);
		/* if close is NULL, closing is a no-op, hence pointless */
		isopen = fp->_close != NULL;
		if ((wantfd = fp->_file) < 0 && isopen) {
			(void) (*fp->_close)(fp->_cookie);
			isopen = 0;
		}
	}

	/* Get a new descriptor to refer to the new file. */
	f = _open(file, oflags, DEFFILEMODE);
	sverrno = errno;

finish:
	/*
	 * Finish closing fp.  Even if the open succeeded above, we cannot
	 * keep fp->_base: it may be the wrong size.  This loses the effect
	 * of any setbuffer calls, but stdio has always done this before.
	 *
	 * Leave the existing file descriptor open until dup2() is called
	 * below to avoid races where a concurrent open() in another thread
	 * could claim the existing descriptor.
	 */
	if (fp->_flags & __SMBF)
		free((char *)fp->_bf._base);
	fp->_w = 0;
	fp->_r = 0;
	fp->_p = NULL;
	fp->_bf._base = NULL;
	fp->_bf._size = 0;
	fp->_lbfsize = 0;
	if (HASUB(fp))
		FREEUB(fp);
	fp->_ub._size = 0;
	if (HASLB(fp))
		FREELB(fp);
	fp->_lb._size = 0;
	fp->_orientation = 0;
	memset(&fp->_mbstate, 0, sizeof(mbstate_t));

	if (f < 0) {			/* did not get it after all */
		if (isopen)
			(void) (*fp->_close)(fp->_cookie);
		fp->_flags = 0;		/* set it free */
		FUNLOCKFILE(fp);
		errno = sverrno;	/* restore in case _close clobbered */
		return (NULL);
	}

	/*
	 * If reopening something that was open before on a real file, try
	 * to maintain the descriptor.  Various C library routines (perror)
	 * assume stderr is always fd STDERR_FILENO, even if being freopen'd.
	 */
	if (wantfd >= 0) {
		if ((oflags & O_CLOEXEC ? _fcntl(f, F_DUP2FD_CLOEXEC, wantfd) :
		    _dup2(f, wantfd)) >= 0) {
			(void)_close(f);
			f = wantfd;
		} else
			(void)_close(fp->_file);
	}

	/*
	 * File descriptors are a full int, but _file is only a short.
	 * If we get a valid file descriptor that is greater than
	 * SHRT_MAX, then the fd will get sign-extended into an
	 * invalid file descriptor.  Handle this case by failing the
	 * open.
	 */
	if (f > SHRT_MAX) {
		fp->_flags = 0;		/* set it free */
		FUNLOCKFILE(fp);
		errno = EMFILE;
		return (NULL);
	}

	fp->_flags = flags;
	fp->_file = f;
	fp->_cookie = fp;
	fp->_read = __sread;
	fp->_write = __swrite;
	fp->_seek = __sseek;
	fp->_close = __sclose;
	/*
	 * When opening in append mode, even though we use O_APPEND,
	 * we need to seek to the end so that ftell() gets the right
	 * answer.  If the user then alters the seek pointer, or
	 * the file extends, this will fail, but there is not much
	 * we can do about this.  (We could set __SAPP and check in
	 * fseek and ftell.)
	 */
	if (oflags & O_APPEND)
		(void) _sseek(fp, (fpos_t)0, SEEK_END);
	FUNLOCKFILE(fp);
	return (fp);
}