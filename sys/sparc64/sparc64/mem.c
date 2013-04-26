
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
 *
 * NOTE: other architectures support mmap()'ing the mem and kmem devices; this
 * might cause illegal aliases to be created for the locked kernel page(s), so
 * it is not implemented.
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
#include <sys/signalvar.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_page.h>
#include <vm/vm_kern.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>

#include <machine/cache.h>
#include <machine/md_var.h>
#include <machine/pmap.h>
#include <machine/tlb.h>

#include <machine/memdev.h>

struct mem_range_softc mem_range_softc;

/* ARGSUSED */
int
memrw(struct cdev *dev, struct uio *uio, int flags)
{
	struct iovec *iov;
	vm_offset_t eva;
	vm_offset_t off;
	vm_offset_t ova;
	vm_offset_t va;
	vm_prot_t prot;
	vm_paddr_t pa;
	vm_size_t cnt;
	vm_page_t m;
	int error;
	int i;
	uint32_t colors;

	cnt = 0;
	colors = 1;
	error = 0;
	ova = 0;

	GIANT_REQUIRED;

	while (uio->uio_resid > 0 && error == 0) {
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			if (uio->uio_iovcnt < 0)
				panic("memrw");
			continue;
		}
		if (dev2unit(dev) == CDEV_MINOR_MEM) {
			pa = uio->uio_offset & ~PAGE_MASK;
			if (!is_physical_memory(pa)) {
				error = EFAULT;
				break;
			}

			off = uio->uio_offset & PAGE_MASK;
			cnt = PAGE_SIZE - ((vm_offset_t)iov->iov_base &
			    PAGE_MASK);
			cnt = ulmin(cnt, PAGE_SIZE - off);
			cnt = ulmin(cnt, iov->iov_len);

			m = NULL;
			for (i = 0; phys_avail[i] != 0; i += 2) {
				if (pa >= phys_avail[i] &&
				    pa < phys_avail[i + 1]) {
					m = PHYS_TO_VM_PAGE(pa);
					break;
				}
			}

			if (m != NULL) {
				if (ova == 0) {
					if (dcache_color_ignore == 0)
						colors = DCACHE_COLORS;
					ova = kmem_alloc_wait(kernel_map,
					    PAGE_SIZE * colors);
				}
				if (colors != 1 && m->md.color != -1)
					va = ova + m->md.color * PAGE_SIZE;
				else
					va = ova;
				pmap_qenter(va, &m, 1);
				error = uiomove((void *)(va + off), cnt,
				    uio);
				pmap_qremove(va, 1);
			} else {
				va = TLB_PHYS_TO_DIRECT(pa);
				error = uiomove((void *)(va + off), cnt,
				    uio);
			}
			break;
		} else if (dev2unit(dev) == CDEV_MINOR_KMEM) {
			va = trunc_page(uio->uio_offset);
			eva = round_page(uio->uio_offset + iov->iov_len);

			/*
			 * Make sure that all of the pages are currently
			 * resident so we don't create any zero fill pages.
			 */
			for (; va < eva; va += PAGE_SIZE)
				if (pmap_kextract(va) == 0)
					return (EFAULT);

			prot = (uio->uio_rw == UIO_READ) ? VM_PROT_READ :
			    VM_PROT_WRITE;
			va = uio->uio_offset;
			if (va < VM_MIN_DIRECT_ADDRESS &&
			    kernacc((void *)va, iov->iov_len, prot) == FALSE)
				return (EFAULT);

			error = uiomove((void *)va, iov->iov_len, uio);
			break;
		}
		/* else panic! */
	}
	if (ova != 0)
		kmem_free_wakeup(kernel_map, ova, PAGE_SIZE * colors);
	return (error);
}