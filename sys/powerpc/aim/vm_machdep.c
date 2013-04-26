
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/malloc.h>
#include <sys/bio.h>
#include <sys/buf.h>
#include <sys/ktr.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/vnode.h>
#include <sys/vmmeter.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/sf_buf.h>
#include <sys/sysctl.h>
#include <sys/sysent.h>
#include <sys/unistd.h>

#include <machine/cpu.h>
#include <machine/fpu.h>
#include <machine/frame.h>
#include <machine/md_var.h>
#include <machine/pcb.h>

#include <dev/ofw/openfirm.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>
#include <vm/vm_extern.h>

/*
 * On systems without a direct mapped region (e.g. PPC64),
 * we use the same code as the Book E implementation. Since
 * we need to have runtime detection of this, define some machinery
 * for sf_bufs in this case, and ignore it on systems with direct maps.
 */

#ifndef NSFBUFS
#define NSFBUFS         (512 + maxusers * 16)
#endif

static void sf_buf_init(void *arg);
SYSINIT(sock_sf, SI_SUB_MBUF, SI_ORDER_ANY, sf_buf_init, NULL);
 
LIST_HEAD(sf_head, sf_buf);
 
/* A hash table of active sendfile(2) buffers */
static struct sf_head *sf_buf_active;
static u_long sf_buf_hashmask;

#define SF_BUF_HASH(m)  (((m) - vm_page_array) & sf_buf_hashmask)

static TAILQ_HEAD(, sf_buf) sf_buf_freelist;
static u_int sf_buf_alloc_want;

/*
 * A lock used to synchronize access to the hash table and free list
 */
static struct mtx sf_buf_lock;

#ifdef __powerpc64__
extern uintptr_t tocbase;
#endif


/*
 * Finish a fork operation, with process p2 nearly set up.
 * Copy and update the pcb, set up the stack so that the child
 * ready to run and return to user mode.
 */
void
cpu_fork(struct thread *td1, struct proc *p2, struct thread *td2, int flags)
{
	struct	trapframe *tf;
	struct	callframe *cf;
	struct	pcb *pcb;

	KASSERT(td1 == curthread || td1 == &thread0,
	    ("cpu_fork: p1 not curproc and not proc0"));
	CTR3(KTR_PROC, "cpu_fork: called td1=%p p2=%p flags=%x",
	    td1, p2, flags);

	if ((flags & RFPROC) == 0)
		return;

	pcb = (struct pcb *)((td2->td_kstack +
	    td2->td_kstack_pages * PAGE_SIZE - sizeof(struct pcb)) & ~0x2fUL);
	td2->td_pcb = pcb;

	/* Copy the pcb */
	bcopy(td1->td_pcb, pcb, sizeof(struct pcb));

	/*
	 * Create a fresh stack for the new process.
	 * Copy the trap frame for the return to user mode as if from a
	 * syscall.  This copies most of the user mode register values.
	 */
	tf = (struct trapframe *)pcb - 1;
	bcopy(td1->td_frame, tf, sizeof(*tf));

	/* Set up trap frame. */
	tf->fixreg[FIRSTARG] = 0;
	tf->fixreg[FIRSTARG + 1] = 0;
	tf->cr &= ~0x10000000;

	td2->td_frame = tf;

	cf = (struct callframe *)tf - 1;
	memset(cf, 0, sizeof(struct callframe));
	#ifdef __powerpc64__
	cf->cf_toc = tocbase;
	#endif
	cf->cf_func = (register_t)fork_return;
	cf->cf_arg0 = (register_t)td2;
	cf->cf_arg1 = (register_t)tf;

	pcb->pcb_sp = (register_t)cf;
	#ifdef __powerpc64__
	pcb->pcb_lr = ((register_t *)fork_trampoline)[0];
	pcb->pcb_toc = ((register_t *)fork_trampoline)[1];
	#else
	pcb->pcb_lr = (register_t)fork_trampoline;
	#endif
	pcb->pcb_cpu.aim.usr_vsid = 0;

	/* Setup to release spin count in fork_exit(). */
	td2->td_md.md_spinlock_count = 1;
	td2->td_md.md_saved_msr = PSL_KERNSET;

	/*
 	 * Now cpu_switch() can schedule the new process.
	 */
}

/*
 * Intercept the return address from a freshly forked process that has NOT
 * been scheduled yet.
 *
 * This is needed to make kernel threads stay in kernel mode.
 */
void
cpu_set_fork_handler(td, func, arg)
	struct thread *td;
	void (*func)(void *);
	void *arg;
{
	struct	callframe *cf;

	CTR4(KTR_PROC, "%s called with td=%p func=%p arg=%p",
	    __func__, td, func, arg);

	cf = (struct callframe *)td->td_pcb->pcb_sp;

	cf->cf_func = (register_t)func;
	cf->cf_arg0 = (register_t)arg;
}

void
cpu_exit(td)
	register struct thread *td;
{
}

/*
 * Allocate a pool of sf_bufs (sendfile(2) or "super-fast" if you prefer. :-))
 */
static void
sf_buf_init(void *arg)
{
        struct sf_buf *sf_bufs;
        vm_offset_t sf_base;
        int i;

	/* Don't bother on systems with a direct map */

	if (hw_direct_map)
		return;

        nsfbufs = NSFBUFS;
        TUNABLE_INT_FETCH("kern.ipc.nsfbufs", &nsfbufs);

        sf_buf_active = hashinit(nsfbufs, M_TEMP, &sf_buf_hashmask);
        TAILQ_INIT(&sf_buf_freelist);
        sf_base = kmem_alloc_nofault(kernel_map, nsfbufs * PAGE_SIZE);
        sf_bufs = malloc(nsfbufs * sizeof(struct sf_buf), M_TEMP, M_NOWAIT | M_ZERO);

        for (i = 0; i < nsfbufs; i++) {
                sf_bufs[i].kva = sf_base + i * PAGE_SIZE;
                TAILQ_INSERT_TAIL(&sf_buf_freelist, &sf_bufs[i], free_entry);
        }
        sf_buf_alloc_want = 0;
        mtx_init(&sf_buf_lock, "sf_buf", NULL, MTX_DEF);
}

/*
 * Get an sf_buf from the freelist. Will block if none are available.
 */
struct sf_buf *
sf_buf_alloc(struct vm_page *m, int flags)
{
        struct sf_head *hash_list;
        struct sf_buf *sf;
        int error;

	if (hw_direct_map) {
		/* Shortcut the direct mapped case */

		return ((struct sf_buf *)m);
	}

        hash_list = &sf_buf_active[SF_BUF_HASH(m)];
        mtx_lock(&sf_buf_lock);
        LIST_FOREACH(sf, hash_list, list_entry) {
                if (sf->m == m) {
                        sf->ref_count++;
                        if (sf->ref_count == 1) {
                                TAILQ_REMOVE(&sf_buf_freelist, sf, free_entry);
                                nsfbufsused++;
                                nsfbufspeak = imax(nsfbufspeak, nsfbufsused);
                        }
                        goto done;
                }
        }

        while ((sf = TAILQ_FIRST(&sf_buf_freelist)) == NULL) {
                if (flags & SFB_NOWAIT)
                        goto done;

                sf_buf_alloc_want++;
                mbstat.sf_allocwait++;
                error = msleep(&sf_buf_freelist, &sf_buf_lock,
                    (flags & SFB_CATCH) ? PCATCH | PVM : PVM, "sfbufa", 0);
                sf_buf_alloc_want--;

                /*
                 * If we got a signal, don't risk going back to sleep.
                 */
                if (error)
                        goto done;
        }

        TAILQ_REMOVE(&sf_buf_freelist, sf, free_entry);
        if (sf->m != NULL)
                LIST_REMOVE(sf, list_entry);

        LIST_INSERT_HEAD(hash_list, sf, list_entry);
        sf->ref_count = 1;
        sf->m = m;
        nsfbufsused++;
        nsfbufspeak = imax(nsfbufspeak, nsfbufsused);
        pmap_qenter(sf->kva, &sf->m, 1);
done:
        mtx_unlock(&sf_buf_lock);
        return (sf);
}

/*
 * Detatch mapped page and release resources back to the system.
 *
 * Remove a reference from the given sf_buf, adding it to the free
 * list when its reference count reaches zero. A freed sf_buf still,
 * however, retains its virtual-to-physical mapping until it is
 * recycled or reactivated by sf_buf_alloc(9).
 */
void
sf_buf_free(struct sf_buf *sf)
{
	if (hw_direct_map)
		return;

        mtx_lock(&sf_buf_lock);
        sf->ref_count--;
        if (sf->ref_count == 0) {
                TAILQ_INSERT_TAIL(&sf_buf_freelist, sf, free_entry);
                nsfbufsused--;

                if (sf_buf_alloc_want > 0)
                        wakeup(&sf_buf_freelist);
        }
        mtx_unlock(&sf_buf_lock);
}

/*
 * Software interrupt handler for queued VM system processing.
 */
void
swi_vm(void *dummy)
{

	if (busdma_swi_pending != 0)
		busdma_swi();
}

/*
 * Tell whether this address is in some physical memory region.
 * Currently used by the kernel coredump code in order to avoid
 * dumping the ``ISA memory hole'' which could cause indefinite hangs,
 * or other unpredictable behaviour.
 */


int
is_physical_memory(addr)
	vm_offset_t addr;
{
	/*
	 * stuff other tests for known memory-mapped devices (PCI?)
	 * here
	 */

	return 1;
}

/*
 * CPU threading functions related to the VM layer. These could be used
 * to map the SLB bits required for the kernel stack instead of forcing a
 * fixed-size KVA.
 */

void
cpu_thread_swapin(struct thread *td)
{
}

void
cpu_thread_swapout(struct thread *td)
{
}