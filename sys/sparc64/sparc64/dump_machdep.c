
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

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>

#include <machine/metadata.h>
#include <machine/kerneldump.h>
#include <machine/ofw_mem.h>
#include <machine/tsb.h>
#include <machine/tlb.h>

CTASSERT(sizeof(struct kerneldumpheader) == DEV_BSIZE);

static struct kerneldumpheader kdh;
static off_t dumplo, dumppos;

/* Handle buffered writes. */
static char buffer[DEV_BSIZE];
static vm_size_t fragsz;

#define	MAXDUMPSZ	(MAXDUMPPGS << PAGE_SHIFT)

static int
buf_write(struct dumperinfo *di, char *ptr, size_t sz)
{
	size_t len;
	int error;

	while (sz) {
		len = DEV_BSIZE - fragsz;
		if (len > sz)
			len = sz;
		bcopy(ptr, buffer + fragsz, len);
		fragsz += len;
		ptr += len;
		sz -= len;
		if (fragsz == DEV_BSIZE) {
			error = dump_write(di, buffer, 0, dumplo,
			    DEV_BSIZE);
			if (error)
				return error;
			dumplo += DEV_BSIZE;
			fragsz = 0;
		}
	}

	return (0);
}

static int
buf_flush(struct dumperinfo *di)
{
	int error;

	if (fragsz == 0)
		return (0);

	error = dump_write(di, buffer, 0, dumplo, DEV_BSIZE);
	dumplo += DEV_BSIZE;
	fragsz = 0;
	return (error);
}

static int
reg_write(struct dumperinfo *di, vm_paddr_t pa, vm_size_t size)
{
	struct sparc64_dump_reg r;

	r.dr_pa = pa;
	r.dr_size = size;
	r.dr_offs = dumppos;
	dumppos += size;
	return (buf_write(di, (char *)&r, sizeof(r)));
}

static int
blk_dump(struct dumperinfo *di, vm_paddr_t pa, vm_size_t size)
{
	vm_size_t pos, rsz;
	vm_offset_t va;
	int c, counter, error, twiddle;

	printf("  chunk at %#lx: %ld bytes ", (u_long)pa, (long)size);

	va = 0L;
	error = counter = twiddle = 0;
	for (pos = 0; pos < size; pos += MAXDUMPSZ, counter++) {
		if (counter % 128 == 0)
			printf("%c\b", "|/-\\"[twiddle++ & 3]);
		rsz = size - pos;
		rsz = (rsz > MAXDUMPSZ) ? MAXDUMPSZ : rsz;
		va = TLB_PHYS_TO_DIRECT(pa + pos);
		error = dump_write(di, (void *)va, 0, dumplo, rsz);
		if (error)
			break;
		dumplo += rsz;

		/* Check for user abort. */
		c = cncheckc();
		if (c == 0x03)
			return (ECANCELED);
		if (c != -1)
			printf("(CTRL-C to abort)  ");
	}
	printf("... %s\n", (error) ? "fail" : "ok");
	return (error);
}

void
dumpsys(struct dumperinfo *di)
{
	struct sparc64_dump_hdr hdr;
	vm_size_t size, totsize, hdrsize;
	int error, i, nreg;

	/* Calculate dump size. */
	size = 0;
	nreg = sparc64_nmemreg;
	for (i = 0; i < sparc64_nmemreg; i++)
		size += sparc64_memreg[i].mr_size;
	/* Account for the header size. */
	hdrsize = roundup2(sizeof(hdr) + sizeof(struct sparc64_dump_reg) * nreg,
	    DEV_BSIZE);
	size += hdrsize;

	totsize = size + 2 * sizeof(kdh);
	if (totsize > di->mediasize) {
		printf("Insufficient space on device (need %ld, have %ld), "
		    "refusing to dump.\n", (long)totsize,
		    (long)di->mediasize);
		error = ENOSPC;
		goto fail;
	}

	/* Determine dump offset on device. */
	dumplo = di->mediaoffset + di->mediasize - totsize;

	mkdumpheader(&kdh, KERNELDUMPMAGIC, KERNELDUMP_SPARC64_VERSION, size,
	    di->blocksize);

	printf("Dumping %lu MB (%d chunks)\n", (u_long)(size >> 20), nreg);

	/* Dump leader */
	error = dump_write(di, &kdh, 0, dumplo, sizeof(kdh));
	if (error)
		goto fail;
	dumplo += sizeof(kdh);

	/* Dump the private header. */
	hdr.dh_hdr_size = hdrsize;
	hdr.dh_tsb_pa = tsb_kernel_phys;
	hdr.dh_tsb_size = tsb_kernel_size;
	hdr.dh_tsb_mask = tsb_kernel_mask;
	hdr.dh_nregions = nreg;

	if (buf_write(di, (char *)&hdr, sizeof(hdr)) != 0)
		goto fail;

	dumppos = hdrsize;
	/* Now, write out the region descriptors. */
	for (i = 0; i < sparc64_nmemreg; i++) {
		error = reg_write(di, sparc64_memreg[i].mr_start,
		    sparc64_memreg[i].mr_size);
		if (error != 0)
			goto fail;
	}
	buf_flush(di);

	/* Dump memory chunks. */
	for (i = 0; i < sparc64_nmemreg; i++) {
		error = blk_dump(di, sparc64_memreg[i].mr_start,
		    sparc64_memreg[i].mr_size);
		if (error != 0)
			goto fail;
	}

	/* Dump trailer */
	error = dump_write(di, &kdh, 0, dumplo, sizeof(kdh));
	if (error)
		goto fail;

	/* Signal completion, signoff and exit stage left. */
	dump_write(di, NULL, 0, 0, 0);
	printf("\nDump complete\n");
	return;

 fail:
	/* XXX It should look more like VMS :-) */
	printf("** DUMP FAILED (ERROR %d) **\n", error);
}