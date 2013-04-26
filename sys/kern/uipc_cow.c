
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

/*
 * This is a set of routines for enabling and disabling copy on write
 * protection for data written into sockets.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysctl.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/mbuf.h>
#include <sys/sf_buf.h>
#include <sys/socketvar.h>
#include <sys/uio.h>

#include <vm/vm.h>
#include <vm/vm_extern.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_page.h>
#include <vm/vm_object.h>

FEATURE(zero_copy_sockets, "Zero copy sockets support");

struct netsend_cow_stats {
	int attempted;
	int fail_not_mapped;
	int fail_sf_buf;
	int success;
	int iodone;
};

static struct netsend_cow_stats socow_stats;

static void socow_iodone(void *addr, void *args);

static void
socow_iodone(void *addr, void *args)
{	
	struct sf_buf *sf;
	vm_page_t pp;

	sf = args;
	pp = sf_buf_page(sf);
	sf_buf_free(sf);
	/* remove COW mapping  */
	vm_page_lock(pp);
	vm_page_cowclear(pp);
	vm_page_unwire(pp, 0);
	/*
	 * Check for the object going away on us. This can
	 * happen since we don't hold a reference to it.
	 * If so, we're responsible for freeing the page.
	 */
	if (pp->wire_count == 0 && pp->object == NULL)
		vm_page_free(pp);
	vm_page_unlock(pp);
	socow_stats.iodone++;
}

int
socow_setup(struct mbuf *m0, struct uio *uio)
{
	struct sf_buf *sf;
	vm_page_t pp;
	struct iovec *iov;
	struct vmspace *vmspace;
	struct vm_map *map;
	vm_offset_t offset, uva;
	vm_size_t len;

	socow_stats.attempted++;
	vmspace = curproc->p_vmspace;
	map = &vmspace->vm_map;
	uva = (vm_offset_t) uio->uio_iov->iov_base;
	offset = uva & PAGE_MASK;
	len = PAGE_SIZE - offset;

	/*
	 * Verify that access to the given address is allowed from user-space.
	 */
	if (vm_fault_quick_hold_pages(map, uva, len, VM_PROT_READ, &pp, 1) <
	    0) {
		socow_stats.fail_not_mapped++;
		return(0);
	}

	/* 
	 * set up COW
	 */
	vm_page_lock(pp);
	if (vm_page_cowsetup(pp) != 0) {
		vm_page_unhold(pp);
		vm_page_unlock(pp);
		return (0);
	}

	/*
	 * wire the page for I/O
	 */
	vm_page_wire(pp);
	vm_page_unhold(pp);
	vm_page_unlock(pp);
	/*
	 * Allocate an sf buf
	 */
	sf = sf_buf_alloc(pp, SFB_CATCH);
	if (sf == NULL) {
		vm_page_lock(pp);
		vm_page_cowclear(pp);
		vm_page_unwire(pp, 0);
		/*
		 * Check for the object going away on us. This can
		 * happen since we don't hold a reference to it.
		 * If so, we're responsible for freeing the page.
		 */
		if (pp->wire_count == 0 && pp->object == NULL)
			vm_page_free(pp);
		vm_page_unlock(pp);
		socow_stats.fail_sf_buf++;
		return(0);
	}
	/* 
	 * attach to mbuf
	 */
	MEXTADD(m0, sf_buf_kva(sf), PAGE_SIZE, socow_iodone,
	    (void*)sf_buf_kva(sf), sf, M_RDONLY, EXT_SFBUF);
	m0->m_len = len;
	m0->m_data = (caddr_t)sf_buf_kva(sf) + offset;
	socow_stats.success++;

	iov = uio->uio_iov;
	iov->iov_base = (char *)iov->iov_base + m0->m_len;
	iov->iov_len -= m0->m_len;
	uio->uio_resid -= m0->m_len;
	uio->uio_offset += m0->m_len;
	if (iov->iov_len == 0) {
		uio->uio_iov++;
		uio->uio_iovcnt--;
	}

	return(m0->m_len);
}