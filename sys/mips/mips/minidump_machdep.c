
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
#include <sys/conf.h>
#include <sys/cons.h>
#include <sys/kernel.h>
#include <sys/kerneldump.h>
#include <sys/msgbuf.h>
#include <vm/vm.h>
#include <vm/pmap.h>
#include <machine/pmap.h>
#include <machine/atomic.h>
#include <machine/elf.h>
#include <machine/md_var.h>
#include <machine/vmparam.h>
#include <machine/minidump.h>
#include <machine/cache.h>

CTASSERT(sizeof(struct kerneldumpheader) == 512);

/*
 * Don't touch the first SIZEOF_METADATA bytes on the dump device. This
 * is to protect us from metadata and to protect metadata from us.
 */
#define	SIZEOF_METADATA		(64*1024)

uint32_t *vm_page_dump;
int vm_page_dump_size;

static struct kerneldumpheader kdh;
static off_t dumplo;
static off_t origdumplo;

/* Handle chunked writes. */
static uint64_t counter, progress;
/* Just auxiliary bufffer */
static char tmpbuffer[PAGE_SIZE];

extern pd_entry_t *kernel_segmap;

CTASSERT(sizeof(*vm_page_dump) == 4);

static int
is_dumpable(vm_paddr_t pa)
{
	int i;

	for (i = 0; dump_avail[i] != 0 || dump_avail[i + 1] != 0; i += 2) {
		if (pa >= dump_avail[i] && pa < dump_avail[i + 1])
			return (1);
	}
	return (0);
}

void
dump_add_page(vm_paddr_t pa)
{
	int idx, bit;

	pa >>= PAGE_SHIFT;
	idx = pa >> 5;		/* 2^5 = 32 */
	bit = pa & 31;
	atomic_set_int(&vm_page_dump[idx], 1ul << bit);
}

void
dump_drop_page(vm_paddr_t pa)
{
	int idx, bit;

	pa >>= PAGE_SHIFT;
	idx = pa >> 5;		/* 2^5 = 32 */
	bit = pa & 31;
	atomic_clear_int(&vm_page_dump[idx], 1ul << bit);
}

#define PG2MB(pgs) (((pgs) + (1 << 8) - 1) >> 8)

static int
write_buffer(struct dumperinfo *di, char *ptr, size_t sz)
{
	size_t len;
	int error, c;
	u_int maxdumpsz;

	maxdumpsz = di->maxiosize;

	if (maxdumpsz == 0)	/* seatbelt */
		maxdumpsz = PAGE_SIZE;

	error = 0;

	while (sz) {
		len = min(maxdumpsz, sz);
		counter += len;
		progress -= len;

		if (counter >> 22) {
			printf(" %jd", PG2MB(progress >> PAGE_SHIFT));
			counter &= (1<<22) - 1;
		}

		if (ptr) {
			error = dump_write(di, ptr, 0, dumplo, len);
			if (error)
				return (error);
			dumplo += len;
			ptr += len;
			sz -= len;
		} else {
			panic("pa is not supported");
		}

		/* Check for user abort. */
		c = cncheckc();
		if (c == 0x03)
			return (ECANCELED);
		if (c != -1)
			printf(" (CTRL-C to abort) ");
	}

	return (0);
}

void
minidumpsys(struct dumperinfo *di)
{
	struct minidumphdr mdhdr;
	uint64_t dumpsize;
	uint32_t ptesize;
	uint32_t bits;
	vm_paddr_t pa;
	vm_offset_t prev_pte = 0;
	uint32_t count = 0;
	vm_offset_t va;
	pt_entry_t *pte;
	int i, bit, error;
	void *dump_va;

	/* Flush cache */
	mips_dcache_wbinv_all();

	counter = 0;
	/* Walk page table pages, set bits in vm_page_dump */
	ptesize = 0;

	for (va = VM_MIN_KERNEL_ADDRESS; va < kernel_vm_end; va += NBPDR) {
		ptesize += PAGE_SIZE;
		pte = pmap_pte(kernel_pmap, va);
		KASSERT(pte != NULL, ("pte for %jx is NULL", (uintmax_t)va));
		for (i = 0; i < NPTEPG; i++) {
			if (pte_test(&pte[i], PTE_V)) {
				pa = TLBLO_PTE_TO_PA(pte[i]);
				if (is_dumpable(pa))
					dump_add_page(pa);
			}
		}
	}

	/*
	 * Now mark pages from 0 to phys_avail[0], that's where kernel 
	 * and pages allocated by pmap_steal reside
	 */
	for (pa = 0; pa < phys_avail[0]; pa += PAGE_SIZE) {
		if (is_dumpable(pa))
			dump_add_page(pa);
	}

	/* Calculate dump size. */
	dumpsize = ptesize;
	dumpsize += round_page(msgbufp->msg_size);
	dumpsize += round_page(vm_page_dump_size);

	for (i = 0; i < vm_page_dump_size / sizeof(*vm_page_dump); i++) {
		bits = vm_page_dump[i];
		while (bits) {
			bit = ffs(bits) - 1;
			pa = (((uint64_t)i * sizeof(*vm_page_dump) * NBBY) +
			    bit) * PAGE_SIZE;
			/* Clear out undumpable pages now if needed */
			if (is_dumpable(pa))
				dumpsize += PAGE_SIZE;
			else
				dump_drop_page(pa);
			bits &= ~(1ul << bit);
		}
	}

	dumpsize += PAGE_SIZE;

	/* Determine dump offset on device. */
	if (di->mediasize < SIZEOF_METADATA + dumpsize + sizeof(kdh) * 2) {
		error = ENOSPC;
		goto fail;
	}

	origdumplo = dumplo = di->mediaoffset + di->mediasize - dumpsize;
	dumplo -= sizeof(kdh) * 2;
	progress = dumpsize;

	/* Initialize mdhdr */
	bzero(&mdhdr, sizeof(mdhdr));
	strcpy(mdhdr.magic, MINIDUMP_MAGIC);
	mdhdr.version = MINIDUMP_VERSION;
	mdhdr.msgbufsize = msgbufp->msg_size;
	mdhdr.bitmapsize = vm_page_dump_size;
	mdhdr.ptesize = ptesize;
	mdhdr.kernbase = VM_MIN_KERNEL_ADDRESS;

	mkdumpheader(&kdh, KERNELDUMPMAGIC, KERNELDUMP_MIPS_VERSION, dumpsize,
	    di->blocksize);

	printf("Physical memory: %ju MB\n", 
	    (uintmax_t)ptoa((uintmax_t)physmem) / 1048576);
	printf("Dumping %llu MB:", (long long)dumpsize >> 20);

	/* Dump leader */
	error = dump_write(di, &kdh, 0, dumplo, sizeof(kdh));
	if (error)
		goto fail;
	dumplo += sizeof(kdh);

	/* Dump my header */
	bzero(tmpbuffer, sizeof(tmpbuffer));
	bcopy(&mdhdr, tmpbuffer, sizeof(mdhdr));
	error = write_buffer(di, tmpbuffer, PAGE_SIZE);
	if (error)
		goto fail;

	/* Dump msgbuf up front */
	error = write_buffer(di, (char *)msgbufp->msg_ptr, 
	    round_page(msgbufp->msg_size));
	if (error)
		goto fail;

	/* Dump bitmap */
	error = write_buffer(di, (char *)vm_page_dump,
	    round_page(vm_page_dump_size));
	if (error)
		goto fail;

	/* Dump kernel page table pages */
	for (va = VM_MIN_KERNEL_ADDRESS; va < kernel_vm_end; va += NBPDR) {
		pte = pmap_pte(kernel_pmap, va);
		KASSERT(pte != NULL, ("pte for %jx is NULL", (uintmax_t)va));
		if (!count) {
			prev_pte = (vm_offset_t)pte;
			count++;
		}
		else {
			if ((vm_offset_t)pte == (prev_pte + count * PAGE_SIZE))
				count++;
			else {
				error = write_buffer(di, (char*)prev_pte,
				    count * PAGE_SIZE);
				if (error)
					goto fail;
				count = 1;
				prev_pte = (vm_offset_t)pte;
			}
		}
	}

	if (count) {
		error = write_buffer(di, (char*)prev_pte, count * PAGE_SIZE);
		if (error)
			goto fail;
		count = 0;
		prev_pte = 0;
	}

	/* Dump memory chunks  page by page*/
	for (i = 0; i < vm_page_dump_size / sizeof(*vm_page_dump); i++) {
		bits = vm_page_dump[i];
		while (bits) {
			bit = ffs(bits) - 1;
			pa = (((uint64_t)i * sizeof(*vm_page_dump) * NBBY) +
			    bit) * PAGE_SIZE;
			dump_va = pmap_kenter_temporary(pa, 0);
			error = write_buffer(di, dump_va, PAGE_SIZE);
			if (error)
				goto fail;
			pmap_kenter_temporary_free(pa);
			bits &= ~(1ul << bit);
		}
	}

	/* Dump trailer */
	error = dump_write(di, &kdh, 0, dumplo, sizeof(kdh));
	if (error)
		goto fail;
	dumplo += sizeof(kdh);

	/* Signal completion, signoff and exit stage left. */
	dump_write(di, NULL, 0, 0, 0);
	printf("\nDump complete\n");
	return;

fail:
	if (error < 0)
		error = -error;

	if (error == ECANCELED)
		printf("\nDump aborted\n");
	else if (error == ENOSPC)
		printf("\nDump failed. Partition too small.\n");
	else
		printf("\n** DUMP FAILED (ERROR %d) **\n", error);
}