
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bio.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/uio.h>

#include <vm/vm.h>
#include <vm/vm_extern.h>

int
physio(struct cdev *dev, struct uio *uio, int ioflag)
{
	struct buf *bp;
	struct cdevsw *csw;
	caddr_t sa;
	u_int iolen;
	int error, i, mapped;

	/* Keep the process UPAGES from being swapped. XXX: why ? */
	PHOLD(curproc);

	bp = getpbuf(NULL);
	sa = bp->b_data;
	error = 0;

	/* XXX: sanity check */
	if(dev->si_iosize_max < PAGE_SIZE) {
		printf("WARNING: %s si_iosize_max=%d, using DFLTPHYS.\n",
		    devtoname(dev), dev->si_iosize_max);
		dev->si_iosize_max = DFLTPHYS;
	}

	for (i = 0; i < uio->uio_iovcnt; i++) {
		while (uio->uio_iov[i].iov_len) {
			bp->b_flags = 0;
			if (uio->uio_rw == UIO_READ) {
				bp->b_iocmd = BIO_READ;
				curthread->td_ru.ru_inblock++;
			} else {
				bp->b_iocmd = BIO_WRITE;
				curthread->td_ru.ru_oublock++;
			}
			bp->b_iodone = bdone;
			bp->b_data = uio->uio_iov[i].iov_base;
			bp->b_bcount = uio->uio_iov[i].iov_len;
			bp->b_offset = uio->uio_offset;
			bp->b_iooffset = uio->uio_offset;
			bp->b_saveaddr = sa;

			/* Don't exceed drivers iosize limit */
			if (bp->b_bcount > dev->si_iosize_max)
				bp->b_bcount = dev->si_iosize_max;

			/* 
			 * Make sure the pbuf can map the request
			 * XXX: The pbuf has kvasize = MAXPHYS so a request
			 * XXX: larger than MAXPHYS - PAGE_SIZE must be
			 * XXX: page aligned or it will be fragmented.
			 */
			iolen = ((vm_offset_t) bp->b_data) & PAGE_MASK;
			if ((bp->b_bcount + iolen) > bp->b_kvasize) {
				bp->b_bcount = bp->b_kvasize;
				if (iolen != 0)
					bp->b_bcount -= PAGE_SIZE;
			}
			bp->b_bufsize = bp->b_bcount;

			bp->b_blkno = btodb(bp->b_offset);

			csw = dev->si_devsw;
			if (uio->uio_segflg == UIO_USERSPACE) {
				if (csw != NULL &&
                                    (csw->d_flags & D_UNMAPPED_IO) != 0)
					mapped = 0;
				else
					mapped = 1;
				if (vmapbuf(bp, mapped) < 0) {
					error = EFAULT;
					goto doerror;
				}
			}

			dev_strategy_csw(dev, csw, bp);
			if (uio->uio_rw == UIO_READ)
				bwait(bp, PRIBIO, "physrd");
			else
				bwait(bp, PRIBIO, "physwr");

			if (uio->uio_segflg == UIO_USERSPACE)
				vunmapbuf(bp);
			iolen = bp->b_bcount - bp->b_resid;
			if (iolen == 0 && !(bp->b_ioflags & BIO_ERROR))
				goto doerror;	/* EOF */
			uio->uio_iov[i].iov_len -= iolen;
			uio->uio_iov[i].iov_base =
			    (char *)uio->uio_iov[i].iov_base + iolen;
			uio->uio_resid -= iolen;
			uio->uio_offset += iolen;
			if( bp->b_ioflags & BIO_ERROR) {
				error = bp->b_error;
				goto doerror;
			}
		}
	}
doerror:
	relpbuf(bp, NULL);
	PRELE(curproc);
	return (error);
}