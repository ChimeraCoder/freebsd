
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
 * Interface to new debugger.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/cons.h>
#include <sys/kdb.h>
#include <sys/pcpu.h>
#include <sys/proc.h>

#include <machine/cpu.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <ddb/ddb.h>

/*
 * Read bytes from kernel address space for debugger.
 */
int
db_read_bytes(vm_offset_t addr, size_t size, char *data)
{
	jmp_buf jb;
	void *prev_jb;
	char *src;
	int ret;

	prev_jb = kdb_jmpbuf(jb);
	ret = setjmp(jb);
	if (ret == 0) {
		src = (char *)addr;
		while (size-- > 0)
			*data++ = *src++;
	}
	(void)kdb_jmpbuf(prev_jb);
	return (ret);
}

/*
 * Write bytes to kernel address space for debugger.
 */
int
db_write_bytes(vm_offset_t addr, size_t size, char *data)
{
	jmp_buf jb;
	void *prev_jb;
	char *dst;
	pt_entry_t *ptep0 = NULL;
	pt_entry_t oldmap0 = 0;
	vm_offset_t addr1;
	pt_entry_t *ptep1 = NULL;
	pt_entry_t oldmap1 = 0;
	int ret;

	prev_jb = kdb_jmpbuf(jb);
	ret = setjmp(jb);
	if (ret == 0) {
		if (addr > trunc_page((vm_offset_t)btext) - size &&
		    addr < round_page((vm_offset_t)etext)) {

			ptep0 = pmap_pte(kernel_pmap, addr);
			oldmap0 = *ptep0;
			*ptep0 |= PG_RW;

			/*
			 * Map another page if the data crosses a page
			 * boundary.
			 */
			if ((*ptep0 & PG_PS) == 0) {
				addr1 = trunc_page(addr + size - 1);
				if (trunc_page(addr) != addr1) {
					ptep1 = pmap_pte(kernel_pmap, addr1);
					oldmap1 = *ptep1;
					*ptep1 |= PG_RW;
				}
			} else {
				addr1 = trunc_4mpage(addr + size - 1);
				if (trunc_4mpage(addr) != addr1) {
					ptep1 = pmap_pte(kernel_pmap, addr1);
					oldmap1 = *ptep1;
					*ptep1 |= PG_RW;
				}
			}

			invltlb();
		}

		dst = (char *)addr;

		while (size-- > 0)
			*dst++ = *data++;
	}

	(void)kdb_jmpbuf(prev_jb);

	if (ptep0) {
		*ptep0 = oldmap0;

		if (ptep1)
			*ptep1 = oldmap1;

		invltlb();
	}

	return (ret);
}

void
db_show_mdpcpu(struct pcpu *pc)
{

	db_printf("APIC ID      = %d\n", pc->pc_apic_id);
	db_printf("currentldt   = 0x%x\n", pc->pc_currentldt);
}