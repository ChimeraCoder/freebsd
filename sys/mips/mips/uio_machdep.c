
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
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/sf_buf.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/vm_param.h>

#include <machine/cache.h>

/*
 * Implement uiomove(9) from physical memory using a combination
 * of the direct mapping and sf_bufs to reduce the creation and
 * destruction of ephemeral mappings.  
 */
int
uiomove_fromphys(vm_page_t ma[], vm_offset_t offset, int n, struct uio *uio)
{
	struct sf_buf *sf;
	struct thread *td = curthread;
	struct iovec *iov;
	void *cp;
	vm_offset_t page_offset;
	vm_paddr_t pa;
	vm_page_t m;
	size_t cnt;
	int error = 0;
	int save = 0;

	KASSERT(uio->uio_rw == UIO_READ || uio->uio_rw == UIO_WRITE,
	    ("uiomove_fromphys: mode"));
	KASSERT(uio->uio_segflg != UIO_USERSPACE || uio->uio_td == curthread,
	    ("uiomove_fromphys proc"));
	save = td->td_pflags & TDP_DEADLKTREAT;
	td->td_pflags |= TDP_DEADLKTREAT;
	while (n > 0 && uio->uio_resid) {
		iov = uio->uio_iov;
		cnt = iov->iov_len;
		if (cnt == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			continue;
		}
		if (cnt > n)
			cnt = n;
		page_offset = offset & PAGE_MASK;
		cnt = ulmin(cnt, PAGE_SIZE - page_offset);
		m = ma[offset >> PAGE_SHIFT];
		pa = VM_PAGE_TO_PHYS(m);
		if (MIPS_DIRECT_MAPPABLE(pa)) {
			sf = NULL;
			cp = (char *)MIPS_PHYS_TO_DIRECT(pa) + page_offset;
			/*
			 * flush all mappings to this page, KSEG0 address first
			 * in order to get it overwritten by correct data
			 */
			mips_dcache_wbinv_range((vm_offset_t)cp, cnt);
			pmap_flush_pvcache(m);
		} else {
			sf = sf_buf_alloc(m, 0);
			cp = (char *)sf_buf_kva(sf) + page_offset;
		}
		switch (uio->uio_segflg) {
		case UIO_USERSPACE:
			maybe_yield();
			if (uio->uio_rw == UIO_READ)
				error = copyout(cp, iov->iov_base, cnt);
			else
				error = copyin(iov->iov_base, cp, cnt);
			if (error) {
				if (sf != NULL)
					sf_buf_free(sf);
				goto out;
			}
			break;
		case UIO_SYSSPACE:
			if (uio->uio_rw == UIO_READ)
				bcopy(cp, iov->iov_base, cnt);
			else
				bcopy(iov->iov_base, cp, cnt);
			break;
		case UIO_NOCOPY:
			break;
		}
		if (sf != NULL)
			sf_buf_free(sf);
		else
			mips_dcache_wbinv_range((vm_offset_t)cp, cnt);
		iov->iov_base = (char *)iov->iov_base + cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= cnt;
		uio->uio_offset += cnt;
		offset += cnt;
		n -= cnt;
	}
out:
	if (save == 0)
		td->td_pflags &= ~TDP_DEADLKTREAT;
	return (error);
}