
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)position.c	8.3 (Berkeley) 4/2/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/mtio.h>

#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>

#include "dd.h"
#include "extern.h"

/*
 * Position input/output data streams before starting the copy.  Device type
 * dependent.  Seekable devices use lseek, and the rest position by reading.
 * Seeking past the end of file can cause null blocks to be written to the
 * output.
 */
void
pos_in(void)
{
	off_t cnt;
	int warned;
	ssize_t nr;
	size_t bcnt;

	/* If known to be seekable, try to seek on it. */
	if (in.flags & ISSEEK) {
		errno = 0;
		if (lseek(in.fd, in.offset * in.dbsz, SEEK_CUR) == -1 &&
		    errno != 0)
			err(1, "%s", in.name);
		return;
	}

	/* Don't try to read a really weird amount (like negative). */
	if (in.offset < 0)
		errx(1, "%s: illegal offset", "iseek/skip");

	/*
	 * Read the data.  If a pipe, read until satisfy the number of bytes
	 * being skipped.  No differentiation for reading complete and partial
	 * blocks for other devices.
	 */
	for (bcnt = in.dbsz, cnt = in.offset, warned = 0; cnt;) {
		if ((nr = read(in.fd, in.db, bcnt)) > 0) {
			if (in.flags & ISPIPE) {
				if (!(bcnt -= nr)) {
					bcnt = in.dbsz;
					--cnt;
				}
			} else
				--cnt;
			continue;
		}

		if (nr == 0) {
			if (files_cnt > 1) {
				--files_cnt;
				continue;
			}
			errx(1, "skip reached end of input");
		}

		/*
		 * Input error -- either EOF with no more files, or I/O error.
		 * If noerror not set die.  POSIX requires that the warning
		 * message be followed by an I/O display.
		 */
		if (ddflags & C_NOERROR) {
			if (!warned) {
				warn("%s", in.name);
				warned = 1;
				summary();
			}
			continue;
		}
		err(1, "%s", in.name);
	}
}

void
pos_out(void)
{
	struct mtop t_op;
	off_t cnt;
	ssize_t n;

	/*
	 * If not a tape, try seeking on the file.  Seeking on a pipe is
	 * going to fail, but don't protect the user -- they shouldn't
	 * have specified the seek operand.
	 */
	if (out.flags & (ISSEEK | ISPIPE)) {
		errno = 0;
		if (lseek(out.fd, out.offset * out.dbsz, SEEK_CUR) == -1 &&
		    errno != 0)
			err(1, "%s", out.name);
		return;
	}

	/* Don't try to read a really weird amount (like negative). */
	if (out.offset < 0)
		errx(1, "%s: illegal offset", "oseek/seek");

	/* If no read access, try using mtio. */
	if (out.flags & NOREAD) {
		t_op.mt_op = MTFSR;
		t_op.mt_count = out.offset;

		if (ioctl(out.fd, MTIOCTOP, &t_op) == -1)
			err(1, "%s", out.name);
		return;
	}

	/* Read it. */
	for (cnt = 0; cnt < out.offset; ++cnt) {
		if ((n = read(out.fd, out.db, out.dbsz)) > 0)
			continue;

		if (n == -1)
			err(1, "%s", out.name);

		/*
		 * If reach EOF, fill with NUL characters; first, back up over
		 * the EOF mark.  Note, cnt has not yet been incremented, so
		 * the EOF read does not count as a seek'd block.
		 */
		t_op.mt_op = MTBSR;
		t_op.mt_count = 1;
		if (ioctl(out.fd, MTIOCTOP, &t_op) == -1)
			err(1, "%s", out.name);

		while (cnt++ < out.offset) {
			n = write(out.fd, out.db, out.dbsz);
			if (n == -1)
				err(1, "%s", out.name);
			if ((size_t)n != out.dbsz)
				errx(1, "%s: write failure", out.name);
		}
		break;
	}
}