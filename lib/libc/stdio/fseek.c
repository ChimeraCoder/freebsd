
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
static char sccsid[] = "@(#)fseek.c	8.3 (Berkeley) 1/2/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "un-namespace.h"
#include "local.h"
#include "libc_private.h"

#define	POS_ERR	(-(fpos_t)1)

int
fseek(FILE *fp, long offset, int whence)
{
	int ret;
	int serrno = errno;

	/* make sure stdio is set up */
	if (!__sdidinit)
		__sinit();

	FLOCKFILE(fp);
	ret = _fseeko(fp, (off_t)offset, whence, 1);
	FUNLOCKFILE(fp);
	if (ret == 0)
		errno = serrno;
	return (ret);
}

int
fseeko(FILE *fp, off_t offset, int whence)
{
	int ret;
	int serrno = errno;

	/* make sure stdio is set up */
	if (!__sdidinit)
		__sinit();

	FLOCKFILE(fp);
	ret = _fseeko(fp, offset, whence, 0);
	FUNLOCKFILE(fp);
	if (ret == 0)
		errno = serrno;
	return (ret);
}

/*
 * Seek the given file to the given offset.
 * `Whence' must be one of the three SEEK_* macros.
 */
int
_fseeko(FILE *fp, off_t offset, int whence, int ltest)
{
	fpos_t (*seekfn)(void *, fpos_t, int);
	fpos_t target, curoff, ret;
	size_t n;
	struct stat st;
	int havepos;

	/*
	 * Have to be able to seek.
	 */
	if ((seekfn = fp->_seek) == NULL) {
		errno = ESPIPE;		/* historic practice */
		return (-1);
	}

	/*
	 * Change any SEEK_CUR to SEEK_SET, and check `whence' argument.
	 * After this, whence is either SEEK_SET or SEEK_END.
	 */
	switch (whence) {

	case SEEK_CUR:
		/*
		 * In order to seek relative to the current stream offset,
		 * we have to first find the current stream offset via
		 * ftell (see ftell for details).
		 */
		if (_ftello(fp, &curoff))
			return (-1);
		if (curoff < 0) {
			/* Unspecified position because of ungetc() at 0 */
			errno = ESPIPE;
			return (-1);
		}
		if (offset > 0 && curoff > OFF_MAX - offset) {
			errno = EOVERFLOW;
			return (-1);
		}
		offset += curoff;
		if (offset < 0) {
			errno = EINVAL;
			return (-1);
		}
		if (ltest && offset > LONG_MAX) {
			errno = EOVERFLOW;
			return (-1);
		}
		whence = SEEK_SET;
		havepos = 1;
		break;

	case SEEK_SET:
		if (offset < 0) {
			errno = EINVAL;
			return (-1);
		}
	case SEEK_END:
		curoff = 0;		/* XXX just to keep gcc quiet */
		havepos = 0;
		break;

	default:
		errno = EINVAL;
		return (-1);
	}

	/*
	 * Can only optimise if:
	 *	reading (and not reading-and-writing);
	 *	not unbuffered; and
	 *	this is a `regular' Unix file (and hence seekfn==__sseek).
	 * We must check __NBF first, because it is possible to have __NBF
	 * and __SOPT both set.
	 */
	if (fp->_bf._base == NULL)
		__smakebuf(fp);
	if (fp->_flags & (__SWR | __SRW | __SNBF | __SNPT))
		goto dumb;
	if ((fp->_flags & __SOPT) == 0) {
		if (seekfn != __sseek ||
		    fp->_file < 0 || _fstat(fp->_file, &st) ||
		    (st.st_mode & S_IFMT) != S_IFREG) {
			fp->_flags |= __SNPT;
			goto dumb;
		}
		fp->_blksize = st.st_blksize;
		fp->_flags |= __SOPT;
	}

	/*
	 * We are reading; we can try to optimise.
	 * Figure out where we are going and where we are now.
	 */
	if (whence == SEEK_SET)
		target = offset;
	else {
		if (_fstat(fp->_file, &st))
			goto dumb;
		if (offset > 0 && st.st_size > OFF_MAX - offset) {
			errno = EOVERFLOW;
			return (-1);
		}
		target = st.st_size + offset;
		if ((off_t)target < 0) {
			errno = EINVAL;
			return (-1);
		}
		if (ltest && (off_t)target > LONG_MAX) {
			errno = EOVERFLOW;
			return (-1);
		}
	}

	if (!havepos && _ftello(fp, &curoff))
		goto dumb;

	/*
	 * (If the buffer was modified, we have to
	 * skip this; see fgetln.c.)
	 */
	if (fp->_flags & __SMOD)
		goto abspos;

	/*
	 * Compute the number of bytes in the input buffer (pretending
	 * that any ungetc() input has been discarded).  Adjust current
	 * offset backwards by this count so that it represents the
	 * file offset for the first byte in the current input buffer.
	 */
	if (HASUB(fp)) {
		curoff += fp->_r;	/* kill off ungetc */
		n = fp->_up - fp->_bf._base;
		curoff -= n;
		n += fp->_ur;
	} else {
		n = fp->_p - fp->_bf._base;
		curoff -= n;
		n += fp->_r;
	}

	/*
	 * If the target offset is within the current buffer,
	 * simply adjust the pointers, clear EOF, undo ungetc(),
	 * and return.
	 */
	if (target >= curoff && target < curoff + n) {
		size_t o = target - curoff;

		fp->_p = fp->_bf._base + o;
		fp->_r = n - o;
		if (HASUB(fp))
			FREEUB(fp);
		fp->_flags &= ~__SEOF;
		memset(&fp->_mbstate, 0, sizeof(mbstate_t));
		return (0);
	}

abspos:
	/*
	 * The place we want to get to is not within the current buffer,
	 * but we can still be kind to the kernel copyout mechanism.
	 * By aligning the file offset to a block boundary, we can let
	 * the kernel use the VM hardware to map pages instead of
	 * copying bytes laboriously.  Using a block boundary also
	 * ensures that we only read one block, rather than two.
	 */
	curoff = target & ~(fp->_blksize - 1);
	if (_sseek(fp, curoff, SEEK_SET) == POS_ERR)
		goto dumb;
	fp->_r = 0;
	fp->_p = fp->_bf._base;
	if (HASUB(fp))
		FREEUB(fp);
	n = target - curoff;
	if (n) {
		if (__srefill(fp) || fp->_r < n)
			goto dumb;
		fp->_p += n;
		fp->_r -= n;
	}
	fp->_flags &= ~__SEOF;
	memset(&fp->_mbstate, 0, sizeof(mbstate_t));
	return (0);

	/*
	 * We get here if we cannot optimise the seek ... just
	 * do it.  Allow the seek function to change fp->_bf._base.
	 */
dumb:
	if (__sflush(fp) ||
	    (ret = _sseek(fp, (fpos_t)offset, whence)) == POS_ERR)
		return (-1);
	if (ltest && ret > LONG_MAX) {
		fp->_flags |= __SERR;
		errno = EOVERFLOW;
		return (-1);
	}
	/* success: clear EOF indicator and discard ungetc() data */
	if (HASUB(fp))
		FREEUB(fp);
	fp->_p = fp->_bf._base;
	fp->_r = 0;
	/* fp->_w = 0; */	/* unnecessary (I think...) */
	fp->_flags &= ~__SEOF;
	memset(&fp->_mbstate, 0, sizeof(mbstate_t));
	return (0);
}