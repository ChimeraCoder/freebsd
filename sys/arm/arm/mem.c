
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
#include <sys/ioccom.h>
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
#include <vm/pmap.h>
#include <vm/vm_extern.h>

#include <machine/memdev.h>
#include <machine/vmparam.h>

/*
 * Used in /dev/mem drivers and elsewhere
 */
MALLOC_DEFINE(M_MEMDESC, "memdesc", "memory range descriptors");

struct mem_range_softc mem_range_softc;

/* ARGSUSED */
int
memrw(struct cdev *dev, struct uio *uio, int flags)
{
	int o;
	u_int c = 0, v;
	struct iovec *iov;
	int error = 0;
	vm_offset_t addr, eaddr;

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
			int i;
			int address_valid = 0;

			v = uio->uio_offset;
			v &= ~PAGE_MASK;
			for (i = 0; dump_avail[i] || dump_avail[i + 1];
			i += 2) {
				if (v >= dump_avail[i] &&
				    v < dump_avail[i + 1]) {
					address_valid = 1;
					break;
				}
			}
			if (!address_valid)
				return (EINVAL);
			pmap_kenter((vm_offset_t)_tmppt, v);
			o = (int)uio->uio_offset & PAGE_MASK;
			c = (u_int)(PAGE_SIZE - ((int)iov->iov_base & PAGE_MASK));
			c = min(c, (u_int)(PAGE_SIZE - o));
			c = min(c, (u_int)iov->iov_len);
			error = uiomove((caddr_t)&_tmppt[o], (int)c, uio);
			pmap_qremove((vm_offset_t)_tmppt, 1);
			continue;
		}
		else if (dev2unit(dev) == CDEV_MINOR_KMEM) {
			c = iov->iov_len;

			/*
			 * Make sure that all of the pages are currently
			 * resident so that we don't create any zero-fill
			 * pages.
			 */
			addr = trunc_page(uio->uio_offset);
			eaddr = round_page(uio->uio_offset + c);

			for (; addr < eaddr; addr += PAGE_SIZE)
				if (pmap_extract(kernel_pmap, addr) == 0)
					return (EFAULT);
			if (!kernacc((caddr_t)(int)uio->uio_offset, c,
			    uio->uio_rw == UIO_READ ?
			    VM_PROT_READ : VM_PROT_WRITE))
#ifdef ARM_USE_SMALL_ALLOC
				if (addr <= VM_MAXUSER_ADDRESS ||
				    addr >= KERNBASE)
#endif
					return (EFAULT);
			error = uiomove((caddr_t)(int)uio->uio_offset, (int)c, uio);
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
/* ARGSUSED */

int
memmmap(struct cdev *dev, vm_ooffset_t offset, vm_paddr_t *paddr,
    int prot __unused, vm_memattr_t *memattr __unused)
{
	if (dev2unit(dev) == CDEV_MINOR_MEM)
		*paddr = offset;
	else if (dev2unit(dev) == CDEV_MINOR_KMEM)
        	*paddr = vtophys(offset);
	/* else panic! */
	return (0);
}