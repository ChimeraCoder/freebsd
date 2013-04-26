
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
#include <sys/exec.h>
#include <sys/imgact.h>
#include <sys/imgact_aout.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mman.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/racct.h>
#include <sys/resourcevar.h>
#include <sys/vnode.h>

#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_extern.h>

#include <compat/svr4/svr4.h>

static int	exec_svr4_imgact(struct image_params *iparams);

static int
exec_svr4_imgact(imgp)
    struct image_params *imgp;
{
    const struct exec *a_out = (const struct exec *) imgp->image_header;
    struct vmspace *vmspace;
    vm_offset_t vmaddr;
    unsigned long virtual_offset, file_offset;
    unsigned long bss_size;
    ssize_t aresid;
    int error;

    if (((a_out->a_magic >> 16) & 0xff) != 0x64)
	return -1;

    /*
     * Set file/virtual offset based on a.out variant.
     */
    switch ((int)(a_out->a_magic & 0xffff)) {
    case 0413:
	virtual_offset = 0;
	file_offset = 1024;
	break;
    case 0314:
	virtual_offset = 4096;
	file_offset = 0;
	break;
    default:
	return (-1);
    }
    bss_size = round_page(a_out->a_bss);
#ifdef DEBUG
    printf("imgact: text: %08lx, data: %08lx, bss: %08lx\n", (u_long)a_out->a_text, (u_long)a_out->a_data, bss_size);
#endif

    /*
     * Check various fields in header for validity/bounds.
     */
    if (a_out->a_entry < virtual_offset ||
	a_out->a_entry >= virtual_offset + a_out->a_text ||
	a_out->a_text & PAGE_MASK || a_out->a_data & PAGE_MASK)
	return (-1);

    /* text + data can't exceed file size */
    if (a_out->a_data + a_out->a_text > imgp->attr->va_size)
	return (EFAULT);
    /*
     * text/data/bss must not exceed limits
     */
    PROC_LOCK(imgp->proc);
    if (a_out->a_text > maxtsiz ||
	a_out->a_data + bss_size > lim_cur(imgp->proc, RLIMIT_DATA) ||
	racct_set(imgp->proc, RACCT_DATA, a_out->a_data + bss_size) != 0) {
    	PROC_UNLOCK(imgp->proc);
	return (ENOMEM);
    }
    PROC_UNLOCK(imgp->proc);

    VOP_UNLOCK(imgp->vp, 0);

    /*
     * Destroy old process VM and create a new one (with a new stack)
     */
    error = exec_new_vmspace(imgp, &svr4_sysvec);
    if (error)
	    goto fail;
    vmspace = imgp->proc->p_vmspace;

    /*
     * Check if file_offset page aligned,.
     * Currently we cannot handle misalinged file offsets,
     * and so we read in the entire image (what a waste).
     */
    if (file_offset & PAGE_MASK) {
#ifdef DEBUG
	printf("imgact: Non page aligned binary %lu\n", file_offset);
#endif
	/*
	 * Map text+data+bss read/write/execute
	 */
	vmaddr = virtual_offset;
	error = vm_map_find(&vmspace->vm_map, NULL, 0, &vmaddr,
		    	    a_out->a_text + a_out->a_data + bss_size, FALSE,
			    VM_PROT_ALL, VM_PROT_ALL, 0);
	if (error)
	    goto fail;

	error = vn_rdwr(UIO_READ, imgp->vp, (void *)vmaddr, file_offset,
	    a_out->a_text + a_out->a_data, UIO_USERSPACE, 0,
	    curthread->td_ucred, NOCRED, &aresid, curthread);
	if (error != 0)
		goto fail;
	if (aresid != 0) {
		error = ENOEXEC;
		goto fail;
	}

	/*
	 * remove write enable on the 'text' part
	 */
	error = vm_map_protect(&vmspace->vm_map,
			       vmaddr,
		   	       vmaddr + a_out->a_text,
		   	       VM_PROT_EXECUTE|VM_PROT_READ,
		   	       TRUE);
	if (error)
	    goto fail;
    }
    else {
#ifdef DEBUG
	printf("imgact: Page aligned binary %lu\n", file_offset);
#endif
	/*
	 * Map text+data read/execute
	 */
	vmaddr = virtual_offset;
	error = vm_mmap(&vmspace->vm_map, &vmaddr,
			a_out->a_text + a_out->a_data,
	    		VM_PROT_READ | VM_PROT_EXECUTE,
	    		VM_PROT_ALL,
	    		MAP_PRIVATE | MAP_FIXED,
			OBJT_VNODE, imgp->vp, file_offset);
	if (error)
	    goto fail;
    
#ifdef DEBUG
	printf("imgact: startaddr=%08lx, length=%08lx\n", (u_long)vmaddr,
	    (u_long)a_out->a_text + a_out->a_data);
#endif
	/*
	 * allow read/write of data
	 */
	error = vm_map_protect(&vmspace->vm_map,
			       vmaddr + a_out->a_text,
			       vmaddr + a_out->a_text + a_out->a_data,
			       VM_PROT_ALL,
			       FALSE);
	if (error)
	    goto fail;
    
	/*
	 * Allocate anon demand-zeroed area for uninitialized data
	 */
	if (bss_size != 0) {
	    vmaddr = virtual_offset + a_out->a_text + a_out->a_data;
	    error = vm_map_find(&vmspace->vm_map, NULL, 0, &vmaddr, 
				bss_size, FALSE, VM_PROT_ALL, VM_PROT_ALL, 0);
	    if (error)
		goto fail;
#ifdef DEBUG
	    printf("imgact: bssaddr=%08lx, length=%08lx\n",
	        (u_long)vmaddr, bss_size);
#endif

	}
    }
    /* Fill in process VM information */
    vmspace->vm_tsize = round_page(a_out->a_text) >> PAGE_SHIFT;
    vmspace->vm_dsize = round_page(a_out->a_data + bss_size) >> PAGE_SHIFT;
    vmspace->vm_taddr = (caddr_t)virtual_offset;
    vmspace->vm_daddr = (caddr_t)virtual_offset + a_out->a_text;

    /* Fill in image_params */
    imgp->interpreted = 0;
    imgp->entry_addr = a_out->a_entry;
    
    imgp->proc->p_sysent = &svr4_sysvec;
fail:
    vn_lock(imgp->vp, LK_EXCLUSIVE | LK_RETRY);
    return (error);
}

/*
 * Tell kern_execve.c about it, with a little help from the linker.
 */
struct execsw svr4_execsw = { exec_svr4_imgact, "svr4 ELF" };
EXEC_SET(execsw_set, svr4_execsw);