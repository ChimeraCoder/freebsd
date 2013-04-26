
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
#include <sys/kernel.h>
#include <sys/stack.h>
#include <sys/sysctl.h>

#include <vm/redzone.h>


static SYSCTL_NODE(_vm, OID_AUTO, redzone, CTLFLAG_RW, NULL, "RedZone data");
static u_long redzone_extra_mem = 0;
SYSCTL_ULONG(_vm_redzone, OID_AUTO, extra_mem, CTLFLAG_RD, &redzone_extra_mem,
    0, "Extra memory allocated by redzone");     
static int redzone_panic = 0;
TUNABLE_INT("vm.redzone.panic", &redzone_panic);
SYSCTL_INT(_vm_redzone, OID_AUTO, panic, CTLFLAG_RW, &redzone_panic, 0,
    "Panic when buffer corruption is detected");     

#define	REDZONE_CHSIZE	(16)
#define	REDZONE_CFSIZE	(16)
#define	REDZONE_HSIZE	(sizeof(struct stack) + sizeof(u_long) + REDZONE_CHSIZE)
#define	REDZONE_FSIZE	(REDZONE_CFSIZE)

static u_long
redzone_roundup(u_long n)
{

	if (n < REDZONE_HSIZE)
		n = REDZONE_HSIZE;
	if (n <= 128)
		return (128);
	else if (n <= 256)
		return (256);
	else if (n <= 512)
		return (512);
	else if (n <= 1024)
		return (1024);
	else if (n <= 2048)
		return (2048);
	return (PAGE_SIZE);
}

u_long
redzone_get_size(caddr_t naddr)
{
	u_long nsize;

	bcopy(naddr - REDZONE_CHSIZE - sizeof(u_long), &nsize, sizeof(nsize));
	return (nsize);
}

u_long
redzone_size_ntor(u_long nsize)
{

	return (nsize + redzone_roundup(nsize) + REDZONE_FSIZE);
}

void *
redzone_addr_ntor(caddr_t naddr)
{

	return (naddr - redzone_roundup(redzone_get_size(naddr)));
}

/*
 * Set redzones and remember allocation backtrace.
 */
void *
redzone_setup(caddr_t raddr, u_long nsize)
{
	struct stack st;
	caddr_t haddr, faddr;

	atomic_add_long(&redzone_extra_mem, redzone_size_ntor(nsize) - nsize);

	haddr = raddr + redzone_roundup(nsize) - REDZONE_HSIZE;
	faddr = haddr + REDZONE_HSIZE + nsize;

	/* Redzone header. */
	stack_save(&st);
	bcopy(&st, haddr, sizeof(st));
	haddr += sizeof(st);
	bcopy(&nsize, haddr, sizeof(nsize));
	haddr += sizeof(nsize);
	memset(haddr, 0x42, REDZONE_CHSIZE);
	haddr += REDZONE_CHSIZE;

	/* Redzone footer. */
	memset(faddr, 0x42, REDZONE_CFSIZE);

	return (haddr);
}

/*
 * Verify redzones.
 * This function is called on free() and realloc().
 */
void
redzone_check(caddr_t naddr)
{
	struct stack ast, fst;
	caddr_t haddr, faddr;
	u_int ncorruptions;
	u_long nsize;
	int i;

	haddr = naddr - REDZONE_HSIZE;
	bcopy(haddr, &ast, sizeof(ast));
	haddr += sizeof(ast);
	bcopy(haddr, &nsize, sizeof(nsize));
	haddr += sizeof(nsize);

	atomic_subtract_long(&redzone_extra_mem,
	    redzone_size_ntor(nsize) - nsize);

	/* Look for buffer underflow. */
	ncorruptions = 0;
	for (i = 0; i < REDZONE_CHSIZE; i++, haddr++) {
		if (*(u_char *)haddr != 0x42)
			ncorruptions++;
	}
	if (ncorruptions > 0) {
		printf("REDZONE: Buffer underflow detected. %u byte%s "
		    "corrupted before %p (%lu bytes allocated).\n",
		    ncorruptions, ncorruptions == 1 ? "" : "s", naddr, nsize);
		printf("Allocation backtrace:\n");
		stack_print_ddb(&ast);
		printf("Free backtrace:\n");
		stack_save(&fst);
		stack_print_ddb(&fst);
		if (redzone_panic)
			panic("Stopping here.");
	}
	faddr = naddr + nsize;
	/* Look for buffer overflow. */
	ncorruptions = 0;
	for (i = 0; i < REDZONE_CFSIZE; i++, faddr++) {
		if (*(u_char *)faddr != 0x42)
			ncorruptions++;
	}
	if (ncorruptions > 0) {
		printf("REDZONE: Buffer overflow detected. %u byte%s corrupted "
		    "after %p (%lu bytes allocated).\n", ncorruptions,
		    ncorruptions == 1 ? "" : "s", naddr + nsize, nsize);
		printf("Allocation backtrace:\n");
		stack_print_ddb(&ast);
		printf("Free backtrace:\n");
		stack_save(&fst);
		stack_print_ddb(&fst);
		if (redzone_panic)
			panic("Stopping here.");
	}
}