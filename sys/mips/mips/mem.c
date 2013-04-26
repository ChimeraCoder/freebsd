
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

/*
 * Memory special file
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/fcntl.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/memrange.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/msgbuf.h>
#include <sys/systm.h>
#include <sys/signalvar.h>
#include <sys/uio.h>

#include <machine/md_var.h>
#include <machine/vmparam.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>
#include <vm/vm_page.h>

#include <machine/memdev.h>

struct mem_range_softc mem_range_softc;

/* ARGSUSED */
int
memrw(struct cdev *dev, struct uio *uio, int flags)
{
	struct iovec *iov;
	int error = 0;
	vm_offset_t va, eva, off, v;
	vm_prot_t prot;
	struct vm_page m;
	vm_page_t marr;
	vm_size_t cnt;

	cnt = 0;
	error = 0;

	GIANT_REQUIRED;

	pmap_page_init(&m);
	while (uio->uio_resid > 0 && !error) {
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			if (uio->uio_iovcnt < 0)
				panic("memrw");
			continue;
		}
		if (dev2unit(dev) == CDEV_MINOR_MEM) {
			v = uio->uio_offset;

			off = uio->uio_offset & PAGE_MASK;
			cnt = PAGE_SIZE - ((vm_offset_t)iov->iov_base &
			    PAGE_MASK);
			cnt = min(cnt, PAGE_SIZE - off);
			cnt = min(cnt, iov->iov_len);

			m.phys_addr = trunc_page(v);
			marr = &m;
			error = uiomove_fromphys(&marr, off, cnt, uio);
		}
		else if (dev2unit(dev) == CDEV_MINOR_KMEM) {
			va = uio->uio_offset;

			va = trunc_page(uio->uio_offset);
			eva = round_page(uio->uio_offset
			    + iov->iov_len);

			/* 
			 * Make sure that all the pages are currently resident
			 * so that we don't create any zero-fill pages.
			 */
			if (va >= VM_MIN_KERNEL_ADDRESS &&
			    eva <= VM_MAX_KERNEL_ADDRESS) {
				for (; va < eva; va += PAGE_SIZE)
					if (pmap_extract(kernel_pmap, va) == 0)
						return (EFAULT);

				prot = (uio->uio_rw == UIO_READ)
				    ? VM_PROT_READ : VM_PROT_WRITE;

				va = uio->uio_offset;
				if (kernacc((void *) va, iov->iov_len, prot)
				    == FALSE)
					return (EFAULT);
			}

			va = uio->uio_offset;
			error = uiomove((void *)va, iov->iov_len, uio);
			continue;
		}
	}

	return (error);
}

/*
 * allow user processes to MMAP some memory sections
 * instead of going through read/write
 */
int
memmmap(struct cdev *dev, vm_ooffset_t offset, vm_paddr_t *paddr,
    int prot, vm_memattr_t *memattr)
{
	/*
	 * /dev/mem is the only one that makes sense through this
	 * interface.  For /dev/kmem any physaddr we return here
	 * could be transient and hence incorrect or invalid at
	 * a later time.
	 */
	if (dev2unit(dev) != CDEV_MINOR_MEM)
		return (-1);

	*paddr = offset;

	return (0);
}