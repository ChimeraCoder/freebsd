
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
#include <sys/sx.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <machine/specialreg.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>

#include <machine/memdev.h>

/*
 * Used in /dev/mem drivers and elsewhere
 */
MALLOC_DEFINE(M_MEMDESC, "memdesc", "memory range descriptors");

static struct sx memsxlock;
SX_SYSINIT(memsxlockinit, &memsxlock, "/dev/mem lock");

/* ARGSUSED */
int
memrw(struct cdev *dev, struct uio *uio, int flags)
{
	int o;
	u_int c = 0;
	vm_paddr_t pa;
	struct iovec *iov;
	int error = 0;
	vm_offset_t addr;

	/* XXX UPS Why ? */
	GIANT_REQUIRED;


	if (dev2unit(dev) != CDEV_MINOR_MEM && dev2unit(dev) != CDEV_MINOR_KMEM)
		return EIO;

	if (dev2unit(dev) == CDEV_MINOR_KMEM && uio->uio_resid > 0) {
		if (uio->uio_offset < (vm_offset_t)VADDR(PTDPTDI, 0))
				return (EFAULT);

		if (!kernacc((caddr_t)(int)uio->uio_offset, uio->uio_resid,
		    uio->uio_rw == UIO_READ ?  VM_PROT_READ : VM_PROT_WRITE))
			return (EFAULT);
	}

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
			pa = uio->uio_offset;
			pa &= ~PAGE_MASK;
		} else {
			/*
			 * Extract the physical page since the mapping may
			 * change at any time. This avoids panics on page 
			 * fault in this case but will cause reading/writing
			 * to the wrong page.
			 * Hopefully an application will notice the wrong
			 * data on read access and refrain from writing.
			 * This should be replaced by a special uiomove
			 * type function that just returns an error if there
			 * is a page fault on a kernel page. 
			 */
			addr = trunc_page(uio->uio_offset);
			pa = pmap_extract(kernel_pmap, addr);
			if (pa == 0) 
				return EFAULT;

		}
		
		/* 
		 * XXX UPS This should just use sf_buf_alloc.
		 * Unfortunately sf_buf_alloc needs a vm_page
		 * and we may want to look at memory not covered
		 * by the page array.
		 */

		sx_xlock(&memsxlock);
		pmap_kenter((vm_offset_t)ptvmmap, pa);
		pmap_invalidate_page(kernel_pmap,(vm_offset_t)ptvmmap);

		o = (int)uio->uio_offset & PAGE_MASK;
		c = PAGE_SIZE - o;
		c = min(c, (u_int)iov->iov_len);
		error = uiomove((caddr_t)&ptvmmap[o], (int)c, uio);
		pmap_qremove((vm_offset_t)ptvmmap, 1);
		sx_xunlock(&memsxlock);
		
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

/*
 * Operations for changing memory attributes.
 *
 * This is basically just an ioctl shim for mem_range_attr_get
 * and mem_range_attr_set.
 */
/* ARGSUSED */
int 
memioctl(struct cdev *dev __unused, u_long cmd, caddr_t data, int flags,
    struct thread *td)
{
	int nd, error = 0;
	struct mem_range_op *mo = (struct mem_range_op *)data;
	struct mem_range_desc *md;
	
	/* is this for us? */
	if ((cmd != MEMRANGE_GET) &&
	    (cmd != MEMRANGE_SET))
		return (ENOTTY);

	/* any chance we can handle this? */
	if (mem_range_softc.mr_op == NULL)
		return (EOPNOTSUPP);

	/* do we have any descriptors? */
	if (mem_range_softc.mr_ndesc == 0)
		return (ENXIO);

	switch (cmd) {
	case MEMRANGE_GET:
		nd = imin(mo->mo_arg[0], mem_range_softc.mr_ndesc);
		if (nd > 0) {
			md = (struct mem_range_desc *)
				malloc(nd * sizeof(struct mem_range_desc),
				       M_MEMDESC, M_WAITOK);
			error = mem_range_attr_get(md, &nd);
			if (!error)
				error = copyout(md, mo->mo_desc, 
					nd * sizeof(struct mem_range_desc));
			free(md, M_MEMDESC);
		}
		else
			nd = mem_range_softc.mr_ndesc;
		mo->mo_arg[0] = nd;
		break;
		
	case MEMRANGE_SET:
		md = (struct mem_range_desc *)malloc(sizeof(struct mem_range_desc),
						    M_MEMDESC, M_WAITOK);
		error = copyin(mo->mo_desc, md, sizeof(struct mem_range_desc));
		/* clamp description string */
		md->mr_owner[sizeof(md->mr_owner) - 1] = 0;
		if (error == 0)
			error = mem_range_attr_set(md, &mo->mo_arg[0]);
		free(md, M_MEMDESC);
		break;
	}
	return (error);
}