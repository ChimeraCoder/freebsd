
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
#include <sys/msgbuf.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <machine/cpu.h>
#include <machine/frame.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>

#include <machine/memdev.h>

struct mem_range_softc mem_range_softc;

static __inline int
ia64_pa_access(vm_offset_t pa)
{
	return (VM_PROT_READ|VM_PROT_WRITE);
}

/* ARGSUSED */
int
memrw(struct cdev *dev, struct uio *uio, int flags)
{
	struct iovec *iov;
	vm_offset_t addr, eaddr, o, v;
	int c, error, rw;

	error = 0;
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
kmemphys:
			/* Allow reads only in RAM. */
			rw = (uio->uio_rw == UIO_READ)
			    ? VM_PROT_READ : VM_PROT_WRITE;
			if ((ia64_pa_access(v) & rw) != rw) {
				error = EFAULT;
				c = 0;
				break;
			}

			o = uio->uio_offset & PAGE_MASK;
			c = min(uio->uio_resid, (int)(PAGE_SIZE - o));
			error = uiomove((caddr_t)IA64_PHYS_TO_RR7(v), c, uio);
			continue;
		}
		else if (dev2unit(dev) == CDEV_MINOR_KMEM) {
			v = uio->uio_offset;

			if (v >= IA64_RR_BASE(6)) {
				v = IA64_RR_MASK(v);
				goto kmemphys;
			}

			c = min(iov->iov_len, MAXPHYS);

			/*
			 * Make sure that all of the pages are currently
			 * resident so that we don't create any zero-fill
			 * pages.
			 */
			addr = trunc_page(v);
			eaddr = round_page(v + c);
			if (addr < VM_MAXUSER_ADDRESS)
				return (EFAULT);
			for (; addr < eaddr; addr += PAGE_SIZE) {
				if (pmap_kextract(addr) == 0)
					return (EFAULT);
			}
			if (!kernacc((caddr_t)v, c, (uio->uio_rw == UIO_READ)
			    ? VM_PROT_READ : VM_PROT_WRITE))
				return (EFAULT);
			error = uiomove((caddr_t)v, c, uio);
			continue;
		}
		/* else panic! */
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

	/*
	 * Allow access only in RAM.
	 */
	if ((prot & ia64_pa_access(atop((vm_offset_t)offset))) != prot)
		return (-1);
	*paddr = IA64_PHYS_TO_RR7(offset);
	return (0);
}