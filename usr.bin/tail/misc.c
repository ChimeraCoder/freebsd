
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

#ifndef lint
static const char sccsid[] = "@(#)misc.c	8.1 (Berkeley) 6/6/93";
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

void
ierr(const char *fname)
{
	warn("%s", fname);
	rval = 1;
}

void
oerr(void)
{
	err(1, "stdout");
}

/*
 * Print `len' bytes from the file associated with `mip', starting at
 * absolute file offset `startoff'. May move map window.
 */
int
mapprint(struct mapinfo *mip, off_t startoff, off_t len)
{
	int n;

	while (len > 0) {
		if (startoff < mip->mapoff || startoff >= mip->mapoff +
		    (off_t)mip->maplen) {
			if (maparound(mip, startoff) != 0)
				return (1);
		}
		n = (mip->mapoff + mip->maplen) - startoff;
		if (n > len)
			n = len;
		WR(mip->start + (startoff - mip->mapoff), n);
		startoff += n;
		len -= n;
	}
	return (0);
}

/*
 * Move the map window so that it contains the byte at absolute file
 * offset `offset'. The start of the map window will be TAILMAPLEN
 * aligned.
 */
int
maparound(struct mapinfo *mip, off_t offset)
{

	if (mip->start != NULL && munmap(mip->start, mip->maplen) != 0)
		return (1);

	mip->mapoff = offset & ~((off_t)TAILMAPLEN - 1);
	mip->maplen = TAILMAPLEN;
	if ((off_t)mip->maplen > mip->maxoff - mip->mapoff)
		mip->maplen = mip->maxoff - mip->mapoff;
	if (mip->maplen <= 0)
		abort();
	if ((mip->start = mmap(NULL, mip->maplen, PROT_READ, MAP_SHARED,
	     mip->fd, mip->mapoff)) == MAP_FAILED)
		return (1);

	return (0);
}